project(ll-toolkit-test-util VERSION 0.1 LANGUAGES CXX)

include(GNUInstallDirs)

add_library(ll-toolkit-test-util INTERFACE)

target_include_directories(ll-toolkit-test-util
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

install(
    TARGETS ll-toolkit-test-util
    EXPORT ll-toolkit-test-util-config
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# This makes the project importable from the install directory
# Put config file in per-project dir (name MUST match), can also
# just go into 'cmake'.
install(EXPORT ll-toolkit-test-util-config DESTINATION ${CMAKE_INSTALL_LIBDIR}/share/cmake)

# This makes the project importable from the build directory
export(TARGETS ll-toolkit-test-util FILE ll-toolkit-test-util-config.cmake)
