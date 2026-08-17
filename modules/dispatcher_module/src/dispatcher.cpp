#include "dispatcher.hpp"
#include "cache_data.hpp"
#include "sensor_data.hpp"

#include <iostream>
#include <chrono>
#include <utility>

namespace dispatcher_mod {
using namespace sensor_data_mod;
using namespace cache_data_mod;

BaseMsgDispatcher::BaseMsgDispatcher(uint32_t sensor_id, DataCache& cache)
    : sensor_id_(sensor_id), cache_ref_(cache)
{}

BaseMsgDispatcher::~BaseMsgDispatcher()
{
    stop();
}

uint32_t BaseMsgDispatcher::get_sensor_id() const
{
    return sensor_id_;
}

void BaseMsgDispatcher::set_ros_callback(BaseMsgDispatcher::RosOutputCallback cb)
{
    ros_cb_ = std::move(cb);
}

void BaseMsgDispatcher::start()
{
    stop_flag_ = false;
    work_thread_ = std::thread([this](){ work_loop(); });
}

void BaseMsgDispatcher::stop()
{
    stop_flag_ = true;
    if (work_thread_.joinable())
    {
        work_thread_.join();
    }
}

// RadarDispatcher
class RadarDispatcher : public BaseMsgDispatcher
{
public:
    RadarDispatcher(uint32_t sensor_id, DataCache& cache)
        : BaseMsgDispatcher(sensor_id, cache)
    {}

    std::string get_disp_type() const override
    {
        return "radar";
    }
protected:
    void work_loop() override
    {
        while (!stop_flag_)
        {
            auto frame_vec = cache_ref_.sort_cache_by_ts(sensor_id_);
            for (auto& ptr : frame_vec)
            {
                if(ptr->get_struct_type() != "radar")
                {
                    continue;
                }
                if(ros_cb_)
                {
                    // Clone数据，不要move缓存内部unique_ptr，防止源数据被掏空
                    auto cloned_data = ptr->clone();
                    ros_cb_(std::move(cloned_data));
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
};

// CameraDispatcher
class CameraDispatcher : public BaseMsgDispatcher
{
public:
    CameraDispatcher(uint32_t sensor_id, DataCache& cache)
        : BaseMsgDispatcher(sensor_id, cache)
    {}

    std::string get_disp_type() const override
    {
        return "camera";
    }
protected:
    void work_loop() override
    {
        while (!stop_flag_)
        {
            auto frame_vec = cache_ref_.sort_cache_by_ts(sensor_id_);
            for (auto& ptr : frame_vec)
            {
                if(ptr->get_struct_type() != "camera")
                {
                    continue;
                }
                if(ros_cb_)
                {
                    auto cloned_data = ptr->clone();
                    ros_cb_(std::move(cloned_data));
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }
    }
};

// ImuDispatcher
class ImuDispatcher : public BaseMsgDispatcher
{
public:
    ImuDispatcher(uint32_t sensor_id, DataCache& cache)
        : BaseMsgDispatcher(sensor_id, cache)
    {}

    std::string get_disp_type() const override
    {
        return "imu";
    }
protected:
    void work_loop() override
    {
        while (!stop_flag_)
        {
            auto frame_vec = cache_ref_.sort_cache_by_ts(sensor_id_);
            for (auto& ptr : frame_vec)
            {
                if(ptr->get_struct_type() != "imu")
                {
                    continue;
                }
                if(ros_cb_)
                {
                    auto cloned_data = ptr->clone();
                    ros_cb_(std::move(cloned_data));
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
};

std::shared_ptr<BaseMsgDispatcher> DispatcherFactory::create(
    const std::string& disp_type,
    uint32_t sensor_id,
    DataCache& cache)
{
    if(disp_type == "radar")
    {
        return std::make_shared<RadarDispatcher>(sensor_id, cache);
    }
    else if(disp_type == "camera")
    {
        return std::make_shared<CameraDispatcher>(sensor_id, cache);
    }
    else if(disp_type == "imu")
    {
        return std::make_shared<ImuDispatcher>(sensor_id, cache);
    }
    std::cerr << "[DispatcherFactory] unknown type:" << disp_type << std::endl;
    return nullptr;
}

} // namespace dispatcher_mod

