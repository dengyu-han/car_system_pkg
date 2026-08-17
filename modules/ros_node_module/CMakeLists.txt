add_library(ros_node_lib STATIC
    src/ros_node.cpp
)
target_link_libraries(ros_node_lib
    dispatcher_lib
    sensor_data_lib
)
target_include_directories(ros_node_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(ros_node_lib rclcpp std_msgs)

