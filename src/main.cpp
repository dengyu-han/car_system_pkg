#include <rclcpp/rclcpp.hpp>
#include <thread>
#include <chrono>
#include <memory>
#include <cassert>
#include <string>

#include "sensor_data.hpp"
#include "cache_data.hpp"
#include "dispatcher.hpp"
#include "logger.hpp"
#include "memory_pool.hpp"
#include "ros_node.hpp"


int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);

    logger_mod::Logger logger;
    logger.init_logger("./sensor_system.log");
    logger.logger_send(logger_mod::LoggerLevel::INFO, "system start");

    memory_pool_mod::RandomMemoryPool mem_pool(64U,200U);
    
    cache_data_mod::DataCache global_cache;

    auto ros_adapter = std::make_shared<ros2_adapter_mod::Ros2SensorAdapter>("sensor_ros_adapter_node");

    auto radar_disp = dispatcher_mod::DispatcherFactory::create("radar", 1U, global_cache);
    auto cam_disp = dispatcher_mod::DispatcherFactory::create("camera", 2U, global_cache);
    auto imu_disp = dispatcher_mod::DispatcherFactory::create("imu", 3U, global_cache);

    ros_adapter->bind_dispatcher_callback([&ros_adapter](sensor_data_mod::SensorDataPtr data){
        ros_adapter->publish_sensor_data(std::move(data));
    });

    radar_disp->set_ros_callback(ros_adapter->disp_cb_);
    cam_disp->set_ros_callback(ros_adapter->disp_cb_);
    imu_disp->set_ros_callback(ros_adapter->disp_cb_);

    radar_disp->start();
    cam_disp->start();
    imu_disp->start();

    std::thread sim_th([&](){
        uint32_t radar_sid = 1U;
        uint32_t cam_sid = 2U;
        uint32_t imu_sid = 3U;
        uint64_t cnt = 0;

        while (rclcpp::ok())
        {
            double ts = static_cast<double>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())) * 1000.0;

            {
                auto radar_ptr = std::make_unique<sensor_data_mod::RadarObstacle>();
                radar_ptr->timestamp = ts;
                radar_ptr->sensor_id = radar_sid;
                radar_ptr->data_len = sizeof(sensor_data_mod::RadarObstacle);
                radar_ptr->x = cnt * 0.2;
                radar_ptr->y = cnt * 0.1;
                radar_ptr->z = 0.3;

                global_cache.set_data(std::move(radar_ptr));
            }

            {
                auto cam_ptr = std::make_unique<sensor_data_mod::CameraData>();
                cam_ptr->timestamp = ts;
                cam_ptr->sensor_id = cam_sid;
                cam_ptr->data_len = sizeof(sensor_data_mod::CameraData);
                cam_ptr->width = 1920.F;
                cam_ptr->height = 1080.F;
                global_cache.set_data(std::move(cam_ptr));
            }

            {
                auto imu_ptr = std::make_unique<sensor_data_mod::ImuData>();
                imu_ptr->timestamp = ts;
                imu_ptr->sensor_id = imu_sid;
                imu_ptr->data_len = sizeof(sensor_data_mod::ImuData);
                imu_ptr->acc_x = 0.1 * cnt;
                imu_ptr->acc_y = 0.05 * cnt;
                imu_ptr->acc_z = 9.8;
                imu_ptr->gyro_x = 0.01;
                imu_ptr->gyro_y = 0.02;
                imu_ptr->gyro_z = 0.0;
                global_cache.set_data(std::move(imu_ptr));
            }

            logger.logger_send(logger_mod::LoggerLevel::INFO, "sim produce frame cnt:" + std::to_string(cnt));
            cnt++;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    });

    rclcpp::spin(ros_adapter);

    rclcpp::shutdown();

    radar_disp->stop();
    cam_disp->stop();
    imu_disp->stop();

    if(sim_th.joinable()){
        sim_th.join();
    }
    logger.stop();
    return 0;
}

