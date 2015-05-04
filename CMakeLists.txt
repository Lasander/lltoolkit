cmake_minimum_required(VERSION 2.8)

project (LLToolkit)

include("gtest.cmake")

set (CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")

cxx_test(dataTests data/unittest/Test_DataModel.cpp gtest_main)
cxx_test(logicTests logic/unittest/Test_StateMachine.cpp gtest_main)

