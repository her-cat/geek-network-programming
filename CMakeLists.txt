cmake_minimum_required(VERSION 3.10)
project(network C)

set(CMAKE_C_STANDARD 99)

# Put the libaries and binaries that get built into directories at the
# top of the build tree rather than in hard-to-find leaf directories.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

#SET(CMAKE_BUILD_TYPE "Debug")
#SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
#SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

include_directories(${PROJECT_SOURCE_DIR})

# check epoll and add config.h for the macro compilation.
include(CheckSymbolExists)

check_symbol_exists(epoll_create "sys/epoll.h" EPOLL_EXISTS)
if (EPOLL_EXISTS)
    message(STATUS "Have epoll")
    add_definitions(-D HAVE_EPOLL)
endif()

check_symbol_exists(poll "sys/poll.h" POLL_EXISTS)
if (POLL_EXISTS)
    message(STATUS "Have poll")
    add_definitions(-D HAVE_POLL)
endif()

check_symbol_exists(select "sys/select.h" SELECT_EXISTS)
if (SELECT_EXISTS)
    message(STATUS "Have select")
    add_definitions(-D HAVE_SELECT)
endif()

add_subdirectory(lib)
add_subdirectory(chap-4)
add_subdirectory(chap-5)
add_subdirectory(chap-6)
add_subdirectory(chap-7)
add_subdirectory(chap-11)
add_subdirectory(chap-12)
add_subdirectory(chap-13)
add_subdirectory(chap-14)
add_subdirectory(chap-15)
add_subdirectory(chap-16)
add_subdirectory(mid-homework)
add_subdirectory(chap-21)
add_subdirectory(chap-22)
add_subdirectory(chap-23)
add_subdirectory(chap-25)
add_subdirectory(chap-26)
