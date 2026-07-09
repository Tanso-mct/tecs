# CMakeLists.txt: main' CMake project. Including the source, define the following:
# The project-specific logic is here.
#
cmake_minimum_required(VERSION 3.8)

# Project name and language
project("main" LANGUAGES CXX)

# ---- Library ----
add_library(main
    # ---- Headers ----


    # ---- Sources ----

)

# Public headers
# - BUILD_INTERFACE: At build time (assuming it is located directly under the source code directory)
# - INSTALL_INTERFACE: After installation (assuming it is placed under "include")
target_include_directories(main
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Precompiled headers
target_precompile_headers(main
    PRIVATE
        "src/pch.h"
)

# Compile options
target_compile_options(main
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# Link library
target_link_libraries(main
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

    add_executable(main_tests
        tests/window_container.cpp
    )

    target_link_libraries(main_tests
        PRIVATE
            tecs
            main
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(main_tests)
endif()

# ---- Installation ----
include(CMakePackageConfigHelpers)

# Install libraries (archives/shared/executable files)
install(TARGETS main
    EXPORT mainTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install the public header
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/include/main/
    DESTINATION include/main
)

# Install the export file (target information)
install(EXPORT mainTargets
    NAMESPACE tecs::game::main::
    DESTINATION lib/cmake/main
)

# Version File Generation
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/mainConfigVersion.cmake"
    VERSION 1.0.0
    COMPATIBILITY AnyNewerVersion
)

# Config File Generation
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/mainConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/mainConfig.cmake"
    @ONLY
)

# Install the Config/Version file
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/mainConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/mainConfigVersion.cmake"
    DESTINATION lib/cmake/main
)