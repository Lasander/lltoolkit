cmake_minimum_required(VERSION 3.9)

if("${CMAKE_EXTRA_GENERATOR}" MATCHES "Eclipse")
    # Define C++14 stardard and llvm runtime to be in use
    # This needs to be before project() definition, see https://public.kitware.com/Bug/view.php?id=15316#c37987
    set(CMAKE_CXX_COMPILER_ARG1 "-std=c++14" CACHE STRING "C++ version for eclipse" FORCE)
    set(CMAKE_ECLIPSE_VERSION "4.4" CACHE STRING "Eclipse version" FORCE)
endif()

# Append common c++ compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Werror")

# Append compiler specific c++ compiler flags
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

add_subdirectory(external)

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
