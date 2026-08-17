#include "cache_data.hpp"

namespace cache_data_mod
{

void DataCache::set_data(SensorDataPtr&& data)
{
    if(!data)
    {
        return;
    }
    uint32_t sid = data->sensor_id;
    std::lock_guard<std::mutex> lock(map_mtx_);

    auto iter = cache_map_.find(sid);
    if(iter == cache_map_.end())
    {
        auto q = std::make_unique<SafeQueueType>(DataCache::QUEUE_MAX_FRAME);
        q->push(std::move(data));
        cache_map_[sid] = std::move(q);
    }
    else
    {
        iter->second->push(std::move(data));
    }
}

std::vector<SensorDataPtr> DataCache::query_frame_by_sensor_id(uint32_t sensor_id) const
{
    std::vector<SensorDataPtr> result;
    std::lock_guard<std::mutex> lock(map_mtx_);

    auto iter = cache_map_.find(sensor_id);
    if(iter == cache_map_.end())
    {
        return result;
    }

    auto& queue_ptr = iter->second;
    auto temp_vec = queue_ptr->get_queue_all_move();

    for(auto& item : temp_vec)
    {
        if(item != nullptr)
        {
            result.push_back(item->clone());
        }
        queue_ptr->push(std::move(item));
    }
    return result;
}

std::vector<SensorDataPtr> DataCache::sort_cache_by_ts(uint32_t sensor_id) const
{
    auto vec = this->query_frame_by_sensor_id(sensor_id);
    std::sort(vec.begin(), vec.end(),
        [](const SensorDataPtr& a, const SensorDataPtr& b)
        {
            return a->timestamp < b->timestamp;
        });
    return vec;
}

}//namespace cache_data_mod

