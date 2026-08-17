#include "ros_node.hpp"
#include "sensor_data.hpp"

namespace ros2_adapter_mod {

Ros2SensorAdapter::Ros2SensorAdapter(const std::string &node_name)
    : rclcpp::Node(node_name)
{
    radar_pub_ = this->create_publisher<std_msgs::msg::Float64MultiArray>("/radar_obstacle", 10);
    camera_pub_ = this->create_publisher<std_msgs::msg::Float64MultiArray>("/camera_data", 10);
    imu_pub_ = this->create_publisher<std_msgs::msg::Float64MultiArray>("/imu_data", 10);
}

void Ros2SensorAdapter::bind_dispatcher_callback(dispatcher_mod::BaseMsgDispatcher::RosOutputCallback callback)
{
    disp_cb_ = std::move(callback);
}

// public对外接口
void Ros2SensorAdapter::publish_sensor_data(sensor_data_mod::SensorDataPtr data)
{
    if (!data)
    {
        return;
    }
    std_msgs::msg::Float64MultiArray out_msg;

    switch (data->sensor_id)
    {
        case 1U:
        {
            auto *radar_data = static_cast<sensor_data_mod::RadarObstacle*>(data.get());
            out_msg.data.push_back(radar_data->timestamp);
            out_msg.data.push_back(static_cast<double>(radar_data->sensor_id));
            out_msg.data.push_back(radar_data->x);
            out_msg.data.push_back(radar_data->y);
            out_msg.data.push_back(radar_data->z);
            radar_pub_->publish(out_msg);
            break;
        }
        case 2U:
        {
            auto *cam_data = static_cast<sensor_data_mod::CameraData*>(data.get());
            out_msg.data.push_back(cam_data->timestamp);
            out_msg.data.push_back(static_cast<double>(cam_data->sensor_id));
            out_msg.data.push_back(static_cast<double>(cam_data->width));
            out_msg.data.push_back(static_cast<double>(cam_data->height));
            camera_pub_->publish(out_msg);
            break;
        }
        case 3U:
        {
            auto *imu_data = static_cast<sensor_data_mod::ImuData*>(data.get());
            out_msg.data.push_back(imu_data->timestamp);
            out_msg.data.push_back(static_cast<double>(imu_data->sensor_id));
            out_msg.data.push_back(imu_data->acc_x);
            out_msg.data.push_back(imu_data->acc_y);
            out_msg.data.push_back(imu_data->acc_z);
            out_msg.data.push_back(imu_data->gyro_x);
            out_msg.data.push_back(imu_data->gyro_y);
            out_msg.data.push_back(imu_data->gyro_z);
            imu_pub_->publish(out_msg);
            break;
        }
        default:
            RCLCPP_WARN(this->get_logger(),"unknown sensor id: %u", data->sensor_id);
            break;
    }
}

} // namespace ros2_adapter_mod

