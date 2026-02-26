# CMakeLists.txt: platform' CMake project. Including the source, define the following:
# The project-specific logic is here.
#
cmake_minimum_required(VERSION 3.8)

# Project name and language
project("platform" LANGUAGES CXX)

# ---- Library ----
add_library(platform
    # ---- Headers ----

    include/platform/platform.h
    include/platform/window_context.h
    include/platform/window_system.h
    include/platform/window_win32.h
    include/platform/window_container.h

    # ---- Sources ----

    src/window_win32.cpp
    src/window_container.cpp
)

# Public headers
# - BUILD_INTERFACE: At build time (assuming it is located directly under the source code directory)
# - INSTALL_INTERFACE: After installation (assuming it is placed under "include")
target_include_directories(platform
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Precompiled headers
target_precompile_headers(platform
    PRIVATE
        "src/pch.h"
)

# Compile options
target_compile_options(platform
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# Link library
target_link_libraries(platform
    PRIVATE
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

    add_executable(platform_tests
        tests/window_container.cpp
    )

    target_link_libraries(platform_tests
        PRIVATE
            tecs
            platform
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(platform_tests)
endif()

# ---- Installation ----
include(CMakePackageConfigHelpers)

# Install libraries (archives/shared/executable files)
install(TARGETS platform
    EXPORT platformTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install the public header
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/include/platform/
    DESTINATION include/platform
)

# Install the export file (target information)
install(EXPORT platformTargets
    NAMESPACE tecs::game::platform::
    DESTINATION lib/cmake/platform
)

# Version File Generation
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/platformConfigVersion.cmake"
    VERSION 1.0.0
    COMPATIBILITY AnyNewerVersion
)

# Config File Generation
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/platformConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/platformConfig.cmake"
    @ONLY
)

# Install the Config/Version file
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/platformConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/platformConfigVersion.cmake"
    DESTINATION lib/cmake/platform
)