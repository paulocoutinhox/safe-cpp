include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
    GIT_SHALLOW TRUE
)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(safe-cpp-tests
    allocation_test.cpp
    arithmetic_test.cpp
    borrow_test.cpp
    bounds_test.cpp
    concurrency_test.cpp
    defer_test.cpp
    diagnostics_test.cpp
    memory_test.cpp
    option_test.cpp
    result_test.cpp
    version_test.cpp
)

target_link_libraries(safe-cpp-tests
    PRIVATE
        safe-cpp::safe-cpp
        GTest::gmock
        GTest::gtest_main
)

if(CMAKE_CXX_STANDARD GREATER_EQUAL 23)
    target_compile_features(safe-cpp-tests PRIVATE cxx_std_23)
else()
    target_compile_features(safe-cpp-tests PRIVATE cxx_std_20)
endif()

include(GoogleTest)
gtest_discover_tests(safe-cpp-tests DISCOVERY_MODE PRE_TEST)
