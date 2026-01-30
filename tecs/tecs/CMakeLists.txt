# CMakeLists.txt: tecs' CMake project. Including the source, define the following:
# The project-specific logic is here.
#
cmake_minimum_required(VERSION 3.8)

# ---- Library ----
add_library(tecs STATIC
    src/tecs.cpp
    include/tecs.h
)

# C++20
target_compile_features(tecs PUBLIC cxx_std_20)

# Public headers
# - BUILD_INTERFACE: At build time (assuming it is located directly under the source code directory)
# - INSTALL_INTERFACE: After installation (assuming it is placed under "include")
target_include_directories(tecs
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
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

    add_executable(tecs_tests
        tests/tecs_test.cpp
    )

    target_link_libraries(tecs_tests
        PRIVATE
            tecs
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(tecs_tests)
endif()

# ---- Installation ----
include(CMakePackageConfigHelpers)

# Install libraries (archives/shared/executable files)
install(TARGETS tecs
    EXPORT tecsTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install the public header
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tecs.h
    DESTINATION include
)

# Install the export file (target information)
install(EXPORT tecsTargets
    NAMESPACE tecs::
    DESTINATION lib/cmake/tecs
)

# Version File Generation
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/tecsConfigVersion.cmake"
    VERSION 1.0.0
    COMPATIBILITY AnyNewerVersion
)

# Generate the config file (using the tecsConfig.cmake.in file described later)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/tecsConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/tecsConfig.cmake"
    @ONLY
)

# Install the Config/Version file
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/tecsConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/tecsConfigVersion.cmake"
    DESTINATION lib/cmake/tecs
)
