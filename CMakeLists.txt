cmake_minimum_required(VERSION 3.9)

project(ll-toolkit VERSION 0.1 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

if("${CMAKE_EXTRA_GENERATOR}" MATCHES "Eclipse")
    # Define C++14 stardard and llvm runtime to be in use
    # This needs to be before project() definition, see https://public.kitware.com/Bug/view.php?id=15316#c37987
    set(CMAKE_CXX_COMPILER_ARG1 "-std=c++17" CACHE STRING "C++ version for eclipse" FORCE)
    set(CMAKE_ECLIPSE_VERSION "4.4" CACHE STRING "Eclipse version" FORCE)

    # Do not generate subprojects
    set(CMAKE_ECLIPSE_GENERATE_LINKED_RESOURCES FALSE CACHE BOOL
        "Do not generate linked resource to the subprojects and to the source files within targets" FORCE)
endif()

# Append common c++ compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Werror")

# Append compiler specific c++ compiler flags
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

find_package(LL-Gtest REQUIRED)

# Testing support for local code
enable_testing()
add_subdirectory(common)
add_subdirectory(data)
add_subdirectory(logic)
add_subdirectory(test_util)

#-------------------------------------------------------------------------------
# Default install path
#-------------------------------------------------------------------------------
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(
        CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install"
        CACHE PATH "Default install path" FORCE)
endif()
