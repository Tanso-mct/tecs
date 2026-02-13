# CMakeLists.txt: Top-level CMake project file. Defines the global configuration.
# Also, include the subproject here.
#
cmake_minimum_required (VERSION 3.8)

# Enable hot reload for the MSVC compiler if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("workspace LANGUAGES CXX")

# Set C++17 as the required standard.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Includes subprojects.
add_subdirectory ("tecs/tecs")
add_subdirectory ("game/game/platform_window")
