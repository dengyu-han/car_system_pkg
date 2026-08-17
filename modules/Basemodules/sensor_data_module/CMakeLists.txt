add_library(sensor_data_lib INTERFACE)
target_include_directories(sensor_data_lib INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

