message(STATUS "Configuring spdlog")

set(SPDLOG_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLES $OFF CACHE BOOL "" FORCE)

add_subdirectory(spdlog)
