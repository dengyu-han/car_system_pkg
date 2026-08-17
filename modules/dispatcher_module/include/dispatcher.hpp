#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>

//Forward declaration resolve circular include
namespace cache_data_mod { class DataCache; }
namespace sensor_data_mod { struct SensorData; using SensorDataPtr = std::unique_ptr<SensorData>; }

namespace dispatcher_mod {

class BaseMsgDispatcher
{
public:
    using RosOutputCallback = std::function<void(sensor_data_mod::SensorDataPtr)>;

    BaseMsgDispatcher(uint32_t sensor_id, cache_data_mod::DataCache& cache);
    virtual ~BaseMsgDispatcher();

    uint32_t get_sensor_id() const;
    void set_ros_callback(RosOutputCallback cb);

    void start();
    void stop();

    virtual std::string get_disp_type() const = 0;

protected:
    virtual void work_loop() = 0;

    uint32_t sensor_id_{0U};
    cache_data_mod::DataCache& cache_ref_;
    RosOutputCallback ros_cb_;

    std::atomic<bool> stop_flag_{false};
    std::thread work_thread_;
};

class DispatcherFactory
{
public:
    static std::shared_ptr<BaseMsgDispatcher> create(
        const std::string& disp_type,
        uint32_t sensor_id,
        cache_data_mod::DataCache& cache
    );
};

} // namespace dispatcher_mod

