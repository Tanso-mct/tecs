# CMakeLists.txt: window' CMake project. Including the source, define the following:
# The project-specific logic is here.
#
cmake_minimum_required(VERSION 3.8)

# Project name and language
project("window" LANGUAGES CXX)

# ---- Library ----
add_library(window
    # ---- Headers ----

    include/window/window.h
    include/window/window_win32.h

    # ---- Sources ----

    src/window_win32.cpp
)

# Public headers
# - BUILD_INTERFACE: At build time (assuming it is located directly under the source code directory)
# - INSTALL_INTERFACE: After installation (assuming it is placed under "include")
target_include_directories(window
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Precompiled headers
target_precompile_headers(window
    PRIVATE
        "src/pch.h"
)

# Compile options
target_compile_options(window
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# Link library
target_link_libraries(window
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

    add_executable(window_tests
        tests/window.cpp
    )

    target_link_libraries(window_tests
        PRIVATE
            tecs
            window
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(window_tests)
endif()

# ---- Installation ----
include(CMakePackageConfigHelpers)

# Install libraries (archives/shared/executable files)
install(TARGETS window
    EXPORT windowTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install the public header
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/include/window/
    DESTINATION include/window
)

# Install the export file (target information)
install(EXPORT windowTargets
    NAMESPACE tecs::game::window::
    DESTINATION lib/cmake/window
)

# Version File Generation
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/windowConfigVersion.cmake"
    VERSION 1.0.0
    COMPATIBILITY AnyNewerVersion
)

# Config File Generation
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/windowConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/windowConfig.cmake"
    @ONLY
)

# Install the Config/Version file
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/windowConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/windowConfigVersion.cmake"
    DESTINATION lib/cmake/window
)