#pragma once

#include <unordered_map>
#include <cstdint>
#include <vector>
#include <memory>
#include <algorithm>

#include "safe_queue.hpp"
#include "sensor_data.hpp"

namespace cache_data_mod
{

using SensorDataPtr = std::unique_ptr<sensor_data_mod::SensorData>;
using SafeQueueType = safe_queue_mod::SafeQueue<SensorDataPtr, safe_queue_mod::QueueFullPolicy::DROP_OLDEST>;

class DataCache
{
public:
    DataCache() = default;
    ~DataCache() = default;

    DataCache(const DataCache&) = delete;
    DataCache& operator=(const DataCache&) = delete;
    DataCache(DataCache&&) = delete;
    DataCache& operator=(DataCache&&) = delete;

    void set_data(SensorDataPtr&& data);

    /// 查询指定传感器历史帧，不消费原始队列，内部clone生成副本
    [[nodiscard]] std::vector<SensorDataPtr> query_frame_by_sensor_id(uint32_t sensor_id) const;

    /// 获取帧并按时间戳升序排序（Day1 lambda+sort）
    [[nodiscard]] std::vector<SensorDataPtr> sort_cache_by_ts(uint32_t sensor_id) const;

private:
    mutable std::mutex map_mtx_;
    std::unordered_map<uint32_t, std::unique_ptr<SafeQueueType>> cache_map_;
    static constexpr std::size_t QUEUE_MAX_FRAME{20U};
};

}//namespace cache_data_mod

