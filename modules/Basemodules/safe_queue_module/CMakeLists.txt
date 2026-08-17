add_library(safe_queue_lib INTERFACE)
target_include_directories(safe_queue_lib INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

