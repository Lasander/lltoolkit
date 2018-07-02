project(ll-toolkit-logic VERSION 0.1 LANGUAGES CXX)

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
    LIBRARY DESTINATION logic/lib
    ARCHIVE DESTINATION logic/lib
    RUNTIME DESTINATION logic/bin
)
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into 'cmake'.
install(EXPORT ll-toolkit-logic-config DESTINATION share/ll-toolkit/cmake)

# This makes the project importable from the build directory
export(TARGETS ll-toolkit-logic FILE ll-toolkit-logic-config.cmake)

add_cxx_test(ll-toolkit-logic-tests
	unittest/Test_StateMachine.cpp
)

target_link_libraries(ll-toolkit-logic-tests
ll-toolkit-test-util
ll-toolkit-logic
)
