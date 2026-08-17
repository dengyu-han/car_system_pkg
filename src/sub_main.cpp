#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/float64_multi_array.hpp>
#include <vector>
#include <cstring>
#include "memory_pool.hpp"
#include "sensor_data.hpp"

class SensorSubNode : public rclcpp::Node
{
public:
    SensorSubNode():Node("sensor_sub_receiver")
    {
        mem_pool_ = std::make_unique<memory_pool_mod::RandomMemoryPool>(64U,100U);

        radar_sub_ = this->create_subscription<std_msgs::msg::Float64MultiArray>(
            "/radar_obstacle",
            10,
            [this](const std_msgs::msg::Float64MultiArray::SharedPtr msg){
                RCLCPP_INFO(this->get_logger(),"[RADAR SEGMENT] size:%zu",msg->data.size());
                do_radar_parse(msg);
            }
        );
        camera_sub_ = this->create_subscription<std_msgs::msg::Float64MultiArray>(
            "/camera_data",
            10,
            [this](const std_msgs::msg::Float64MultiArray::SharedPtr msg){
                RCLCPP_INFO(this->get_logger(),"[CAMERA SEGMENT] size:%zu",msg->data.size());
                do_camera_parse(msg);
            }
        );
        imu_sub_ = this->create_subscription<std_msgs::msg::Float64MultiArray>(
            "/imu_data",
            10,
            [this](const std_msgs::msg::Float64MultiArray::SharedPtr msg){
                RCLCPP_INFO(this->get_logger(),"[IMU SEGMENT] size:%zu",msg->data.size());
                do_imu_parse(msg);
            }
        );
    }

private:
    std::vector<std::vector<double>> split_data_segment(const std::vector<double>& src, size_t frame_size)
    {
        std::vector<std::vector<double>> res;
        if(frame_size == 0 || src.empty()) return res;
        for(size_t i = 0; i + frame_size <= src.size(); i += frame_size)
        {
            std::vector<double> seg(src.begin()+i, src.begin()+i+frame_size);
            res.push_back(std::move(seg));
        }
        return res;
    }

    void do_radar_parse(const std_msgs::msg::Float64MultiArray::SharedPtr& msg)
    {
        const auto& src_vec = msg->data;
        size_t byte_cnt = src_vec.size() * sizeof(double);
        //==== 使用 allocate_by_bytes ====
        void* buf = mem_pool_->allocate_by_bytes(byte_cnt);
        if(nullptr == buf)
        {
            RCLCPP_WARN(get_logger(),"radar pool allocate fail");
            return;
        }
        std::memcpy(buf, src_vec.data(), byte_cnt);

        auto seg_list = split_data_segment(src_vec,5U);
        for(auto& seg : seg_list)
        {
            if(seg.size() !=5) continue;
            RCLCPP_INFO(get_logger(),"radar ts:%.2lf, sid:%0.f x:%.2lf y:%.2lf z:%.2lf",
                seg[0],seg[1],seg[2],seg[3],seg[4]);
        }
        mem_pool_->deallocate(buf);
    }

    void do_camera_parse(const std_msgs::msg::Float64MultiArray::SharedPtr& msg)
    {
        const auto& src_vec = msg->data;
        size_t byte_cnt = src_vec.size() * sizeof(double);
        //==== 使用 allocate_by_bytes ====
        void* buf = mem_pool_->allocate_by_bytes(byte_cnt);
        if(nullptr == buf)
        {
            RCLCPP_WARN(get_logger(),"camera pool allocate fail");
            return;
        }
        std::memcpy(buf, src_vec.data(), byte_cnt);

        auto seg_list = split_data_segment(src_vec,4U);
        for(auto& seg : seg_list)
        {
            if(seg.size() !=4) continue;
            RCLCPP_INFO(get_logger(),"camera ts:%.2lf sid:%.0f w:%.0f h:%.0f",
                seg[0],seg[1],seg[2],seg[3]);
        }
        mem_pool_->deallocate(buf);
    }

    void do_imu_parse(const std_msgs::msg::Float64MultiArray::SharedPtr& msg)
    {
        const auto& src_vec = msg->data;
        size_t byte_cnt = src_vec.size() * sizeof(double);
        //==== 使用 allocate_by_bytes ====
        void* buf = mem_pool_->allocate_by_bytes(byte_cnt);
        if(nullptr == buf)
        {
            RCLCPP_WARN(get_logger(),"imu pool allocate fail");
            return;
        }
        std::memcpy(buf, src_vec.data(), byte_cnt);

        auto seg_list = split_data_segment(src_vec,8U);
        for(auto& seg : seg_list)
        {
            if(seg.size() !=8) continue;
            RCLCPP_INFO(get_logger(),"imu ts:%.2lf sid:%.0f acc_z:%.2lf",
                seg[0],seg[1],seg[4]);
        }
        mem_pool_->deallocate(buf);
    }

    std::unique_ptr<memory_pool_mod::RandomMemoryPool> mem_pool_;
    rclcpp::Subscription<std_msgs::msg::Float64MultiArray>::SharedPtr radar_sub_;
    rclcpp::Subscription<std_msgs::msg::Float64MultiArray>::SharedPtr camera_sub_;
    rclcpp::Subscription<std_msgs::msg::Float64MultiArray>::SharedPtr imu_sub_;
};

int main(int argc,char** argv)
{
    rclcpp::init(argc,argv);
    auto node = std::make_shared<SensorSubNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}

