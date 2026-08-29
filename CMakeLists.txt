cmake_minimum_required(VERSION 3.20)

project(
    safe-cpp
    VERSION 0.1.0
    DESCRIPTION "Safety-oriented building blocks for modern C++"
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(SAFE_CPP_BUILD_TESTS "Build safe-cpp tests" OFF)
option(SAFE_CPP_BUILD_EXAMPLES "Build safe-cpp examples" OFF)
option(SAFE_CPP_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(SAFE_CPP_ENABLE_THREAD_SANITIZER "Enable ThreadSanitizer" OFF)
option(SAFE_CPP_ENABLE_COVERAGE "Enable coverage instrumentation" OFF)

add_library(safe-cpp
    src/allocation/allocation.cpp
    src/arithmetic/arithmetic.cpp
    src/borrow/borrow.cpp
    src/bounds/bounds.cpp
    src/concurrency/concurrency.cpp
    src/defer/defer.cpp
    src/diagnostics/diagnostics.cpp
    src/memory/memory.cpp
    src/option/option.cpp
    src/result/result.cpp
    src/version/version.cpp
)

add_library(safe-cpp::safe-cpp ALIAS safe-cpp)

target_compile_features(safe-cpp PUBLIC cxx_std_20)
target_include_directories(safe-cpp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(safe-cpp PROPERTIES
    CXX_EXTENSIONS OFF
    EXPORT_NAME safe-cpp
)

if(MSVC)
    target_compile_options(safe-cpp PUBLIC /Zc:__cplusplus)
    target_compile_options(safe-cpp PRIVATE /W4 /permissive-)
else()
    target_compile_options(safe-cpp PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

if(SAFE_CPP_ENABLE_SANITIZERS AND SAFE_CPP_ENABLE_THREAD_SANITIZER)
    message(FATAL_ERROR "SAFE_CPP_ENABLE_SANITIZERS and SAFE_CPP_ENABLE_THREAD_SANITIZER cannot be enabled together")
endif()

if(SAFE_CPP_ENABLE_SANITIZERS AND NOT MSVC)
    target_compile_options(safe-cpp PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(safe-cpp PUBLIC -fsanitize=address,undefined)
endif()

if(SAFE_CPP_ENABLE_THREAD_SANITIZER AND NOT MSVC)
    target_compile_options(safe-cpp PUBLIC -fsanitize=thread -fno-omit-frame-pointer)
    target_link_options(safe-cpp PUBLIC -fsanitize=thread)
endif()

if(SAFE_CPP_ENABLE_COVERAGE AND NOT MSVC)
    target_compile_options(safe-cpp PUBLIC --coverage -fno-omit-frame-pointer)
    target_link_options(safe-cpp PUBLIC --coverage)
endif()

if(SAFE_CPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(SAFE_CPP_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()

install(
    TARGETS safe-cpp
    EXPORT safe-cpp-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
    EXPORT safe-cpp-targets
    FILE safe-cpp-targets.cmake
    NAMESPACE safe-cpp::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/safe-cpp
)

configure_package_config_file(
    cmake/safe-cpp-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/safe-cpp-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/safe-cpp
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/safe-cpp-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/safe-cpp-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/safe-cpp-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/safe-cpp
)
