cmake_minimum_required(VERSION 2.8)

if("${CMAKE_EXTRA_GENERATOR}" MATCHES "Eclipse")
    # Define C++14 stardard and llvm runtime to be in use
    # This needs to be before project() definition, see https://public.kitware.com/Bug/view.php?id=15316#c37987
    set(CMAKE_CXX_COMPILER_ARG1 "-std=c++14" CACHE STRING "C++ version for eclipse" FORCE)
    set(CMAKE_ECLIPSE_VERSION "4.4" CACHE STRING "Eclipse version" FORCE)
endif()

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

# Append common c++ compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Werror")

# Append compiler specific c++ compiler flags
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

# Testing support
enable_testing()

add_subdirectory(external)

add_subdirectory(common)
add_subdirectory(data)
add_subdirectory(logic)

# add the executable
add_executable(lltoolkit main.cpp)

# add the install targets
install (TARGETS lltoolkit DESTINATION bin)
install (FILES "${PROJECT_BINARY_DIR}/LLToolkitConfig.hpp" DESTINATION include)

# # add_subdirectory(doc)
