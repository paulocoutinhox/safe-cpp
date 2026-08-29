#include <safe_cpp/concurrency/channel.hpp>
#include <safe_cpp/concurrency/mutex.hpp>
#include <safe_cpp/diagnostics/diagnostics.hpp>

#include <gtest/gtest.h>

#include <atomic>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace safe::concurrency::tests
{
TEST(ConcurrencyTest, MutexProtectsAccess)
{
    Mutex<int> value(0);

    // clang-format off
    value.withLock([](int& number)
    {
        number = 42;
    });

    const auto result = value.withLock([](const int& number)
    {
        return number;
    });
    // clang-format on

    EXPECT_EQ(result, 42);
}

TEST(ConcurrencyTest, ChannelTransfersValues)
{
    auto [sender, receiver] = Channel<std::string>::create();
    EXPECT_TRUE(sender.send("hello"));

    auto value = receiver.receive();
    EXPECT_EQ(value.value(), "hello");
}

TEST(ConcurrencyTest, ChannelTryReceiveIsEmptyUntilSend)
{
    auto [sender, receiver] = Channel<int>::create();
    EXPECT_FALSE(receiver.tryReceive());

    EXPECT_TRUE(sender.send(7));
    EXPECT_EQ(receiver.tryReceive().value(), 7);
}

TEST(ConcurrencyTest, ClosedChannelRejectsNewValues)
{
    auto [sender, receiver] = Channel<int>::create();
    sender.close();

    EXPECT_FALSE(sender.send(42));
    EXPECT_FALSE(receiver.receive());
}

TEST(ConcurrencyTest, ClosedChannelDrainsRemainingValues)
{
    auto [sender, receiver] = Channel<int>::create();
    EXPECT_TRUE(sender.send(1));
    EXPECT_TRUE(sender.send(2));
    sender.close();

    EXPECT_EQ(receiver.receive().value(), 1);
    EXPECT_EQ(receiver.receive().value(), 2);
    EXPECT_FALSE(receiver.receive());
}

TEST(ConcurrencyTest, ChannelErrorHasMessage)
{
    EXPECT_EQ(channelErrorMessage(ChannelError::closed), "Channel is closed");
}

TEST(ConcurrencyTest, ClosedChannelTryReceiveDrainsRemainingValues)
{
    auto [sender, receiver] = Channel<int>::create();
    EXPECT_TRUE(sender.send(1));
    EXPECT_TRUE(sender.send(2));
    sender.close();

    EXPECT_EQ(receiver.tryReceive().value(), 1);
    EXPECT_EQ(receiver.tryReceive().value(), 2);
    EXPECT_FALSE(receiver.tryReceive());
    EXPECT_TRUE(receiver.isClosed());
}

TEST(ConcurrencyTest, RejectsMovedFromChannelEndpoints)
{
    auto [sender, receiver] = Channel<int>::create();
    auto movedSender = std::move(sender);
    auto movedReceiver = std::move(receiver);

    EXPECT_TRUE(movedSender.send(1));
    EXPECT_EQ(movedReceiver.tryReceive().value(), 1);
    EXPECT_THROW(static_cast<void>(sender.send(2)), diagnostics::Violation);
    EXPECT_THROW(static_cast<void>(receiver.tryReceive()), diagnostics::Violation);
    EXPECT_THROW(sender.close(), diagnostics::Violation);
    EXPECT_THROW(static_cast<void>(receiver.isClosed()), diagnostics::Violation);
}

TEST(ConcurrencyTest, ChannelReportsClosedState)
{
    auto [sender, receiver] = Channel<int>::create();
    EXPECT_FALSE(sender.isClosed());
    EXPECT_FALSE(receiver.isClosed());

    sender.close();

    EXPECT_TRUE(sender.isClosed());
    EXPECT_TRUE(receiver.isClosed());
    EXPECT_FALSE(receiver.tryReceive());
}

TEST(ConcurrencyTest, MutexProtectsConcurrentIncrements)
{
    Mutex<int> value(0);
    constexpr int workerCount = 8;
    constexpr int incrementsPerWorker = 200;
    std::vector<std::thread> workers;
    workers.reserve(static_cast<std::size_t>(workerCount));

    for (int worker = 0; worker < workerCount; ++worker)
    {
        // clang-format off
        workers.emplace_back([&value]()
        {
            for (int step = 0; step < incrementsPerWorker; ++step)
            {
                value.withLock([](int& number)
                {
                    ++number;
                });
            }
        });
        // clang-format on
    }

    for (auto &worker : workers)
    {
        worker.join();
    }

    // clang-format off
    const auto result = value.withLock([](const int& number)
    {
        return number;
    });
    // clang-format on

    EXPECT_EQ(result, workerCount * incrementsPerWorker);
}

TEST(ConcurrencyTest, ChannelTransfersAcrossThreads)
{
    auto [sender, receiver] = Channel<int>::create();
    constexpr int messageCount = 128;

    // clang-format off
    std::thread producer([sender, messageCount]()
    {
        for (int value = 0; value < messageCount; ++value)
        {
            EXPECT_TRUE(sender.send(value));
        }

        sender.close();
    });
    // clang-format on

    int received = 0;
    while (receiver.receive())
    {
        ++received;
    }

    producer.join();
    EXPECT_EQ(received, messageCount);
}

TEST(ConcurrencyTest, ChannelAcceptsConcurrentProducers)
{
    auto [sender, receiver] = Channel<int>::create();
    constexpr int producerCount = 4;
    constexpr int messagesPerProducer = 64;
    std::vector<std::thread> producers;
    producers.reserve(static_cast<std::size_t>(producerCount));

    for (int producer = 0; producer < producerCount; ++producer)
    {
        // clang-format off
        producers.emplace_back([sender, producer]()
        {
            for (int value = 0; value < messagesPerProducer; ++value)
            {
                EXPECT_TRUE(sender.send((producer * messagesPerProducer) + value));
            }
        });
        // clang-format on
    }

    for (auto &worker : producers)
    {
        worker.join();
    }

    sender.close();

    int received = 0;
    while (receiver.receive())
    {
        ++received;
    }

    EXPECT_EQ(received, producerCount * messagesPerProducer);
}

TEST(ConcurrencyTest, ChannelDrainsWithConcurrentConsumers)
{
    auto [sender, receiver] = Channel<int>::create();
    constexpr int consumerCount = 4;
    constexpr int messageCount = 256;
    std::atomic<int> received{0};
    std::vector<std::thread> consumers;
    consumers.reserve(static_cast<std::size_t>(consumerCount));

    for (int consumer = 0; consumer < consumerCount; ++consumer)
    {
        // clang-format off
        consumers.emplace_back([receiver, &received]()
        {
            while (receiver.receive())
            {
                received.fetch_add(1, std::memory_order_relaxed);
            }
        });
        // clang-format on
    }

    // clang-format off
    std::thread producer([sender, messageCount]()
    {
        for (int value = 0; value < messageCount; ++value)
        {
            EXPECT_TRUE(sender.send(value));
        }

        sender.close();
    });
    // clang-format on

    producer.join();

    for (auto &worker : consumers)
    {
        worker.join();
    }

    EXPECT_EQ(received.load(), messageCount);
}
} // namespace safe::concurrency::tests
