# CMakeLists.txt: tecs' CMake project. Including the source, define the following:
# The project-specific logic is here.
#

# Add the source to this project's executable file.
add_executable (tecs "tecs.cpp" "tecs.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET tecs PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if necessary.
