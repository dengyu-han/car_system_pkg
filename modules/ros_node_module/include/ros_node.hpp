#pragma once

#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float64_multi_array.hpp>
#include <vector>

#include "sensor_data.hpp"
#include "dispatcher.hpp"

namespace ros2_adapter_mod {

class Ros2SensorAdapter : public rclcpp::Node
{
public:
    explicit Ros2SensorAdapter(const std::string &node_name);

    void bind_dispatcher_callback(dispatcher_mod::BaseMsgDispatcher::RosOutputCallback callback);
    //对外公开接口，main可以调用
    void publish_sensor_data(sensor_data_mod::SensorDataPtr data);

    dispatcher_mod::BaseMsgDispatcher::RosOutputCallback disp_cb_;

private:
    rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr radar_pub_;
    rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr camera_pub_;
    rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr imu_pub_;
    
};

} // namespace ros2_adapter_mod

