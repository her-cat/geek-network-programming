cmake_minimum_required(VERSION 3.10)
project(network C)

set(CMAKE_C_STANDARD 99)

# Put the libaries and binaries that get built into directories at the
# top of the build tree rather than in hard-to-find leaf directories.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

# SET(CMAKE_BUILD_TYPE "Debug")
# SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
# SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

add_subdirectory(chap-4)
add_subdirectory(chap-5)
add_subdirectory(chap-6)
add_subdirectory(chap-7)
add_subdirectory(chap-11)
add_subdirectory(chap-12)
add_subdirectory(chap-13)
add_subdirectory(chap-14)
