add_library(cache_data_lib STATIC
    src/cache_data.cpp
)
target_link_libraries(cache_data_lib
    safe_queue_lib
    sensor_data_lib
)
target_include_directories(cache_data_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

