# CMakeLists.txt: platform_window' CMake project. Including the source, define the following:
# The project-specific logic is here.
#
cmake_minimum_required(VERSION 3.8)

# Project name and language
project("platform_window" LANGUAGES CXX)

# ---- Library ----
add_library(platform_window
    # ---- Headers ----

    include/platform_window/game.h

    # ---- Sources ----

    src/game.cpp
)

# Public headers
# - BUILD_INTERFACE: At build time (assuming it is located directly under the source code directory)
# - INSTALL_INTERFACE: After installation (assuming it is placed under "include")
target_include_directories(platform_window
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Precompiled headers
target_precompile_headers(platform_window
    PRIVATE
        "src/pch.h"
)

# Compile options
target_compile_options(platform_window
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# Link library
target_link_libraries(platform_window
    PUBLIC
        tecs
)

# ---- Tests ----
include(CTest)  # BUILD_TESTING option + enable_testing()

if (BUILD_TESTING)
    include(FetchContent)

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(platform_window_tests
        tests/test.cpp
    )

    target_link_libraries(platform_window_tests
        PRIVATE
            platform_window
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(platform_window_tests)
endif()

# ---- Installation ----
include(CMakePackageConfigHelpers)

# Install libraries (archives/shared/executable files)
install(TARGETS platform_window
    EXPORT platform_windowTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install the public header
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/include/platform_window/
    DESTINATION include/platform_window
)

# Install the export file (target information)
install(EXPORT platform_windowTargets
    NAMESPACE platform_window::
    DESTINATION lib/cmake/platform_window
)

# Version File Generation
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/platform_windowConfigVersion.cmake"
    VERSION 1.0.0
    COMPATIBILITY AnyNewerVersion
)

# Config File Generation
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/platform_windowConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/platform_windowConfig.cmake"
    @ONLY
)

# Install the Config/Version file
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/platform_windowConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/platform_windowConfigVersion.cmake"
    DESTINATION lib/cmake/platform_window
)