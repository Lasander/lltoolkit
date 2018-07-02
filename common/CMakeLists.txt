project(ll-toolkit-common VERSION 0.1 LANGUAGES CXX)

add_library(ll-toolkit-common INTERFACE)

target_include_directories(ll-toolkit-common
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

install(
    TARGETS ll-toolkit-common
    EXPORT ll-toolkit-common-config
    LIBRARY DESTINATION common/lib
    ARCHIVE DESTINATION common/lib
    RUNTIME DESTINATION common/bin
)
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into 'cmake'.
install(EXPORT ll-toolkit-common-config DESTINATION share/ll-toolkit/cmake)

# This makes the project importable from the build directory
export(TARGETS ll-toolkit-common FILE ll-toolkit-common-config.cmake)

 add_cxx_test(ll-toolkit-common-tests
	unittest/Test_Synchronized.cpp
	unittest/Test_TypeHelpers.cpp
)

target_link_libraries(ll-toolkit-common-tests
    ll-toolkit-common
)