cmake_minimum_required(VERSION 3.9)

project(ll-toolkit-logic VERSION 0.1 LANGUAGES CXX)

include(GNUInstallDirs)

add_library(ll-toolkit-logic INTERFACE)

target_include_directories(ll-toolkit-logic
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# target_compile_features(ll-toolkit-data
#     PUBLIC
#         cxx_auto_type
#     PRIVATE
#         cxx_variadic_templates)

install(
    TARGETS ll-toolkit-logic
    EXPORT ll-toolkit-logic-config
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT ll-toolkit-logic-config DESTINATION share/ll-toolkit/cmake)
export(TARGETS ll-toolkit-logic FILE ll-toolkit-logic-config.cmake)

add_cxx_test(ll-toolkit-logic-tests
	unittest/Test_StateMachine.cpp
)

target_link_libraries(ll-toolkit-logic-tests
    ll-toolkit-logic
    ll-toolkit-test-util
)
