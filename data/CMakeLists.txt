project(ll-toolkit-data VERSION 0.1 LANGUAGES CXX)

add_library(ll-toolkit-data
    src/CascadingConfigurationRead.cpp
    src/Configuration.cpp
)

target_include_directories(ll-toolkit-data
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        src
)

# target_compile_features(ll-toolkit-data
#     PUBLIC
#         cxx_auto_type
#     PRIVATE
#         cxx_variadic_templates)

target_link_libraries(ll-toolkit-data
    ll-toolkit-common
)

install(
    TARGETS ll-toolkit-data
    EXPORT ll-toolkit-data-config
    LIBRARY DESTINATION data/lib
    ARCHIVE DESTINATION data/lib
    RUNTIME DESTINATION data/bin
)
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into 'cmake'.
install(EXPORT ll-toolkit-data-config DESTINATION share/ll-toolkit/cmake)

# This makes the project importable from the build directory
export(TARGETS ll-toolkit-data FILE ll-toolkit-data-config.cmake)

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
    ll-toolkit-test-util
)
