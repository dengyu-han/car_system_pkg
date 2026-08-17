add_library(dispatcher_lib STATIC
    src/dispatcher.cpp
)
target_link_libraries(dispatcher_lib
    safe_queue_lib
    sensor_data_lib
    cache_data_lib
    logger_lib
)
target_include_directories(dispatcher_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

