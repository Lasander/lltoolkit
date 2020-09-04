###############################################################################
#

if(NOT TARGET LL::GTest)
    find_package(GTest QUIET)

    if(NOT GTest_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        release-1.10.0
        )
        FetchContent_GetProperties(googletest)
        if(NOT googletest_POPULATED)
            FetchContent_Populate(googletest)
            add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})

            add_library(GTest::gtest ALIAS gtest)
            add_library(GTest::gtest_main ALIAS gtest_main)
            add_library(GTest::gmock ALIAS gmock)
        endif()
    endif()

    add_library(LL-GTest INTERFACE IMPORTED GLOBAL)
    target_link_libraries(LL-GTest
        INTERFACE
            GTest::gtest
            GTest::gtest_main
            GTest::gmock
    )

    add_library(LL::GTest ALIAS LL-GTest)
endif()

# Function to add googletest executable targets to project
#
# add_gtest(<name> source1 [source2 ...])
#
function(add_gtest name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name}
        PRIVATE
            LL::GTest
    )
    add_test(NAME ${name} COMMAND ${name})
    install(TARGETS ${name} DESTINATION bin/tests)
endfunction(add_gtest)
