
# find_package(Protobuf REQUIRED)
# include_directories(${PROTOBUF_INCLUDE_DIRS})
# protobuf_generate_cpp(DATA_PROTO_SRCS DATA_PROTO_HDRS unittest/Test.proto)

set(dataSources
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
install (FILES ${dataSources} DESTINATION include/data)

set(dataTestSources 
    ${dataSources}
	unittest/Test_ConcreteQueue.cpp
	unittest/Test_Data.cpp
	unittest/Test_DataModel.cpp
	unittest/Test_Configuration.cpp
	unittest/Test_HeterogeneousQueue.cpp
	unittest/Test_HeterogeneousRingBuffer.cpp
	# ${DATA_PROTO_SRCS} ${DATA_PROTO_HDRS}
)

add_cxx_test(dataTests "${dataTestSources}")
