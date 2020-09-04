cmake_minimum_required(VERSION 3.9)

project(ll-toolkit-data VERSION 0.1 LANGUAGES CXX)

include(GNUInstallDirs)

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
    PUBLIC
        ll-toolkit-common
)

install(
    TARGETS ll-toolkit-data
    EXPORT ll-toolkit-data-config
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT ll-toolkit-data-config DESTINATION ${CMAKE_INSTALL_LIBDIR}/share)
export(TARGETS ll-toolkit-data FILE ll-toolkit-data-config.cmake)

add_gtest(ll-toolkit-data-tests
    unittest/Test_ConcreteQueue.cpp
    unittest/Test_Data.cpp
    unittest/Test_DataModel.cpp
    unittest/Test_Configuration.cpp
    unittest/Test_HeterogeneousQueue.cpp
    unittest/Test_HeterogeneousRingBuffer.cpp
)

target_link_libraries(ll-toolkit-data-tests
    PRIVATE
        ll-toolkit-data
        ll-toolkit-test-util
)
