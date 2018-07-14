message(STATUS "Configuring googletest")
# Prevent overriding the parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(googletest)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(gtest
        PUBLIC -Wno-unused-function
    )
endif()

# Function to add googletest executable targets to project
#
# add_cxx_test(<name> source1 [source2 ...])
#
function(add_cxx_test name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PUBLIC gmock gtest gtest_main)
    add_test(NAME ${name} COMMAND ${name})
    install(TARGETS ${name} DESTINATION bin/tests)
endfunction(add_cxx_test)
