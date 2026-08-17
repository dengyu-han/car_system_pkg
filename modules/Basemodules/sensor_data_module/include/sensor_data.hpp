#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <string>

namespace sensor_data_mod
{

/**
 * @brief 传感器数据基类
 */
struct SensorData
{
    double timestamp{0.0};          ///< 时间戳 ms
    uint32_t sensor_id{0U};         ///< 传感器ID
    size_t data_len{0U};            ///< 数据字节长度

    virtual ~SensorData() = default;
    [[nodiscard]] virtual std::string get_struct_type() const = 0;
    virtual std::unique_ptr<SensorData> clone() const = 0;
};

/**
 * @brief 雷达障碍物数据，继承SensorData
 */
struct RadarObstacle : public SensorData
{
    double x{0.0};
    double y{0.0};
    double z{0.0};

    [[nodiscard]] std::string get_struct_type() const override
    {
        return "radar";
    }

    std::unique_ptr<SensorData> clone() const override
    {
        auto obj = std::make_unique<RadarObstacle>();
        obj->timestamp = this->timestamp;
        obj->sensor_id = this->sensor_id;
        obj->data_len = this->data_len;
        obj->x = this->x;
        obj->y = this->y;
        obj->z = this->z;
        return obj;
    }
};

/**
 * @brief 相机帧数据
 */
struct CameraData : public SensorData
{
    float width{0.F};
    float height{0.F};
    std::vector<uint8_t> image_buf;

    [[nodiscard]] std::string get_struct_type() const override
    {
        return "camera";
    }

    std::unique_ptr<SensorData> clone() const override
    {
        auto obj = std::make_unique<CameraData>();
        obj->timestamp = this->timestamp;
        obj->sensor_id = this->sensor_id;
        obj->data_len = this->data_len;
        obj->width = this->width;
        obj->height = this->height;
        obj->image_buf = this->image_buf;
        return obj;
    }
};

/**
 * @brief IMU惯性测量单元数据
 */
struct ImuData : public SensorData
{
    double acc_x{0.0};
    double acc_y{0.0};
    double acc_z{0.0};

    double gyro_x{0.0};
    double gyro_y{0.0};
    double gyro_z{0.0};

    [[nodiscard]] std::string get_struct_type() const override
    {
        return "imu";
    }

    std::unique_ptr<SensorData> clone() const override
    {
        auto obj = std::make_unique<ImuData>();
        obj->timestamp = this->timestamp;
        obj->sensor_id = this->sensor_id;
        obj->data_len = this->data_len;
        obj->acc_x = this->acc_x;
        obj->acc_y = this->acc_y;
        obj->acc_z = this->acc_z;
        obj->gyro_x = this->gyro_x;
        obj->gyro_y = this->gyro_y;
        obj->gyro_z = this->gyro_z;
        return obj;
    }
};

/// 智能指针别名
using SensorDataPtr = std::unique_ptr<SensorData>;

} 

