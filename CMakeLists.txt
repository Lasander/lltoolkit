cmake_minimum_required(VERSION 2.8)
set(CMAKE_ECLIPSE_VERSION "4.4" CACHE INTERNAL "Eclipse version" FORCE)

project (LLToolkit)

set (USE_GMOCK 1)
include("cmake/gtest.cmake")

set (CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")

#add_executable(main main.cpp)
#target_link_libraries(main nanomsg)
#target_include_directories(main PUBLIC $<INSTALL_INTERFACE:include/nanomsg>)

#add_executable(cloneserver cloneserver.cpp)
#target_link_libraries(cloneserver nanomsg)
#target_include_directories(cloneserver PUBLIC $<INSTALL_INTERFACE:include/nanomsg>)

#add_executable(cloneclient cloneclient.cpp)
#target_link_libraries(cloneclient nanomsg)
#target_include_directories(cloneclient PUBLIC $<INSTALL_INTERFACE:include/nanomsg>)

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

# Common
set(commonTestSources 
	common/TypeHelpers.hpp
	common/unittest/LogHelpers.hpp
	common/unittest/Test_TypeHelpers.cpp
)
cxx_test(commonTests "${commonTestSources}" gtest_main)

# Data
protobuf_generate_cpp(DATA_PROTO_SRCS DATA_PROTO_HDRS data/unittest/Test.proto)
set(dataTestSources
    data/DataReadIf.hpp
    data/DataWriteIf.hpp
    data/DataIf.hpp
    data/Data.hpp
    data/DataModelReadIf.hpp
    data/DataModelIf.hpp
    data/DataModel.hpp
    data/Publisher.hpp
    data/SerializableIf.hpp
	data/ConcreteQueue.hpp
    data/ConfigurationReadIf.hpp
    data/ConfigurationWriteIf.hpp
    data/ConfigurationIf.hpp
    data/Configuration.hpp
    data/Configuration.cpp
    data/CascadingConfigurationRead.hpp
    data/CascadingConfigurationRead.cpp
	data/HeterogeneousQueue.hpp
	data/HeterogeneousRingBuffer.hpp
	data/unittest/Test_ConcreteQueue.cpp
	data/unittest/Test_Data.cpp
	data/unittest/Test_DataModel.cpp
	data/unittest/Test_Configuration.cpp
	data/unittest/Test_HeterogeneousQueue.cpp
	data/unittest/Test_HeterogeneousRingBuffer.cpp
	${DATA_PROTO_SRCS} ${DATA_PROTO_HDRS}
)
cxx_test(dataTests "${dataTestSources}" gtest_main ${PROTOBUF_LIBRARIES})

# Logic
set(logicTestSources 
	logic/StateMachine.hpp
	logic/unittest/Test_StateMachine.cpp
)
cxx_test(logicTests "${logicTestSources}" gtest_main)

