#include"safe_queue.hpp"
#include<gtest/gtest.h>
#include<vector>
#include<thread>
#include<atomic>
using namespace safe_queue_mod;

TEST(SafeQueueTest, PushPopNomal){
    //====单线测试====
    SafeQueue<int, QueueFullPolicy::BLOCK_WAIT> queue(5);

    ASSERT_TRUE(queue.push(100));
    ASSERT_TRUE(queue.push(200));
    int val=0;
    ASSERT_TRUE(queue.pop(val));
    ASSERT_EQ(val,100);
    ASSERT_TRUE(queue.pop(val));
    ASSERT_EQ(val,200);
}

TEST(SafeQueueTest, StopPushAndPop){
    //====停工测试====
    SafeQueue<int, QueueFullPolicy::BLOCK_WAIT> queue(5);
    queue.stop();
    ASSERT_FALSE(queue.push(20));
    int val=0;
    ASSERT_FALSE(queue.pop(val));
}

TEST(SafeQueueTest, ResetPushAndPop){
    //=====恢复已经停工的====
    SafeQueue<int, QueueFullPolicy::BLOCK_WAIT> queue(5);
    queue.stop();
    ASSERT_FALSE(queue.push(100));
    int val=0;
    ASSERT_FALSE(queue.pop(val));

    queue.reset();
    ASSERT_TRUE(queue.push(200));
    int outdata=0;
    ASSERT_TRUE(queue.pop(outdata));
    ASSERT_EQ(outdata,200);
}

TEST(SafeQueueTest, DROPOLDEST){
    //====丢弃最旧数据====
    SafeQueue<int, QueueFullPolicy::DROP_OLDEST> queue(3);
    queue.push(10);
    queue.push(20);
    queue.push(30);
    queue.push(40);

    int val=0;
    queue.pop(val); ASSERT_EQ(val,20);
    queue.pop(val); ASSERT_EQ(val,30);
    queue.pop(val); ASSERT_EQ(val,40);
}

TEST(SafeQueueTest,BlockWaitPushTime){
    //====超时测试====
    SafeQueue<int, QueueFullPolicy::BLOCK_WAIT> queue(1);
    queue.push(100); //队列已满 后续不pop再push就会等待超时
    auto time=std::chrono::milliseconds(50);
    bool ok=queue.push(20,time);
    ASSERT_FALSE(ok);
}

TEST(SafeQueueTest,StopWakeConsumerSleep){
    //====停工唤醒阻塞测试====消费者
    SafeQueue<int, QueueFullPolicy::BLOCK_WAIT> queue(5);
    std::thread th([&queue](){   //捕获引用 生成子线程
        int val=0;
        bool ok=queue.pop(val);
        ASSERT_FALSE(ok);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    queue.stop();
    th.join();
}

TEST(SafeQueueTest,StopWakeProducerSleep){
    //====停工唤醒阻塞测试====生产者
    SafeQueue<int, QueueFullPolicy::BLOCK_WAIT> queue(1);
    ASSERT_TRUE(queue.push(10));
    std::thread th([&queue](){
        bool ok=queue.push(100,std::chrono::milliseconds(300));
        ASSERT_FALSE(ok);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    queue.stop();
    th.join();
}

TEST(SafeQueueTest, MultiProducerMultiConsumer){
    constexpr size_t queue_max_size_=20;
    SafeQueue<int, QueueFullPolicy::BLOCK_WAIT> queue(queue_max_size_);
    const int producer_cnt=4;
    const int consumer_cnt=4;
    const int per_produce=100;

    std::vector<std::thread> producers_;
    std::vector<std::thread> consumers_;
    std::atomic<int> totally_consumed{0};

    for(int i=0;i<producer_cnt;++i){
        producers_.emplace_back([&queue,i,per_produce](){
            for(int j=0;j<per_produce;j++){
                int data=100*i+j;
                while(!queue.push(data, std::chrono::milliseconds(100))){
                    if(queue.is_stopped()) break;
                }
            }
        });
    }

    for(int i=0; i<consumer_cnt;++i){
        //捕获&queue，&totally_consumed
        consumers_.emplace_back([&queue,&totally_consumed](){
            int val{};
            while(queue.pop(val,std::chrono::milliseconds(100))){
                totally_consumed.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    for(auto& p : producers_){
        p.join();
    }

    queue.stop();
    for(auto& c :consumers_){
        c.join();
    }

    int expect_total=producer_cnt * per_produce;
    ASSERT_EQ(expect_total,totally_consumed.load());
}

int main(int argc,char ** argv){
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}
