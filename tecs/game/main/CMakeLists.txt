# CMakeLists.txt: main' CMake project. Including the source, define the following:
# The project-specific logic is here.
#
cmake_minimum_required(VERSION 3.8)

# Project name and language
project("main" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Executable ----
add_executable(main
    src/main.cpp
)

# C++ 17 standard
target_compile_features(main
    PUBLIC
        cxx_std_17
)

# Public headers
target_include_directories(main
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
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

if(WIN32)
    target_link_options(main
        PRIVATE
            $<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>
            $<$<CONFIG:Release>:/ENTRY:mainCRTStartup>
            
            $<$<CONFIG:Debug>:/SUBSYSTEM:CONSOLE>
    )
endif()

# ---- Installation ----
install(TARGETS main
    RUNTIME DESTINATION bin
)