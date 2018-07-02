cmake_minimum_required(VERSION 3.9)

project(ll-toolkit-common VERSION 0.1 LANGUAGES CXX)

include(GNUInstallDirs)

add_library(ll-toolkit-common INTERFACE)

target_include_directories(ll-toolkit-common
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

install(
    TARGETS ll-toolkit-common
    EXPORT ll-toolkit-common-config
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT ll-toolkit-common-config DESTINATION ${CMAKE_INSTALL_LIBDIR}/share)
export(TARGETS ll-toolkit-common FILE ll-toolkit-common-config.cmake)

 add_cxx_test(ll-toolkit-common-tests
	unittest/Test_Synchronized.cpp
	unittest/Test_TypeHelpers.cpp
)

target_link_libraries(ll-toolkit-common-tests
    ll-toolkit-common
)