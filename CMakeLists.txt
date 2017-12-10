cmake_minimum_required(VERSION 2.8)
set(CMAKE_ECLIPSE_VERSION "4.4" CACHE INTERNAL "Eclipse version" FORCE)

project(LLToolkit)

# The version number.
set(LLToolkit_VERSION_MAJOR 0)
set(LLToolkit_VERSION_MINOR 1)

include(CMakeToolsHelpers OPTIONAL)

# configure a header file to pass some of the CMake settings to the source code
configure_file(
    "${PROJECT_SOURCE_DIR}/LLToolkitConfig.hpp.in"
    "${PROJECT_BINARY_DIR}/LLToolkitConfig.hpp"
)
# add the binary tree to the search path for include files
# so that we will find the xxConfig.h
include_directories("${PROJECT_BINARY_DIR}")

# Append c++ compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -stdlib=libc++ -Wall -Werror")

# Testing support
enable_testing()
include("cmake/gtest.cmake")

add_subdirectory(common) 
add_subdirectory(data) 
add_subdirectory(logic) 

# add the executable
add_executable(lltoolkit main.cpp)

# add the install targets
install (TARGETS lltoolkit DESTINATION bin)
install (FILES "${PROJECT_BINARY_DIR}/LLToolkitConfig.hpp" DESTINATION include)

# # add_subdirectory(doc)
