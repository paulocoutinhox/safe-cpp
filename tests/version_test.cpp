#include <safe_cpp/version/version.hpp>

#include <gtest/gtest.h>

namespace safe::version::tests
{
TEST(VersionTest, ExposesLibraryVersion)
{
    EXPECT_EQ(string(), "0.1.0");
    EXPECT_GE(languageLevel(), 20);
}
} // namespace safe::version::tests
