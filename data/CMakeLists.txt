cmake_minimum_required(VERSION 3.9)

# find_package(Protobuf REQUIRED)
# include_directories(${PROTOBUF_INCLUDE_DIRS})
# protobuf_generate_cpp(DATA_PROTO_SRCS DATA_PROTO_HDRS unittest/Test.proto)

project(ll-toolkit-data LANGUAGES CXX)

add_library(ll-toolkit-data
    DataReadIf.hpp
    DataWriteIf.hpp
    DataIf.hpp
    Data.hpp
    DataModelReadIf.hpp
    DataModelIf.hpp
    DataModel.hpp
    Publisher.hpp
    SerializableIf.hpp
	ConcreteQueue.hpp
    ConfigurationReadIf.hpp
    ConfigurationWriteIf.hpp
    ConfigurationIf.hpp
    Configuration.hpp
    Configuration.cpp
    CascadingConfigurationRead.hpp
    CascadingConfigurationRead.cpp
	HeterogeneousQueue.hpp
	HeterogeneousRingBuffer.hpp
)

set(PUBLIC_HEADERS
    DataReadIf.hpp
    DataWriteIf.hpp
    DataIf.hpp
    Data.hpp
    DataModelReadIf.hpp
    DataModelIf.hpp
    DataModel.hpp
    Publisher.hpp
    SerializableIf.hpp
    ConcreteQueue.hpp
    ConfigurationReadIf.hpp
    ConfigurationWriteIf.hpp
    ConfigurationIf.hpp
    Configuration.hpp
    CascadingConfigurationRead.hpp
    HeterogeneousQueue.hpp
    HeterogeneousRingBuffer.hpp
)
set_target_properties(ll-toolkit-data
    PROPERTIES PUBLIC_HEADER
        "${PUBLIC_HEADERS}"
)

INSTALL(
    TARGETS
        ll-toolkit-data
    LIBRARY DESTINATION
        data/lib
    ARCHIVE DESTINATION
        data/lib
    PUBLIC_HEADER DESTINATION
        data/include
)

add_cxx_test(ll-toolkit-data-tests
    unittest/Test_ConcreteQueue.cpp
    unittest/Test_Data.cpp
    unittest/Test_DataModel.cpp
    unittest/Test_Configuration.cpp
    unittest/Test_HeterogeneousQueue.cpp
    unittest/Test_HeterogeneousRingBuffer.cpp
)

target_link_libraries(ll-toolkit-data-tests
    ll-toolkit-data
)
