#pragma once
#include<mutex>
#include<condition_variable>
#include<queue>
#include<cstddef>
#include<chrono>
#include<cstdint>
#include<cassert>
#include<vector>

namespace safe_queue_mod{
enum class QueueFullPolicy{
BLOCK_WAIT,
DROP_OLDEST
};

template<typename T, QueueFullPolicy Policy>
class SafeQueue{
private:
mutable std::mutex mtx_;
std::condition_variable producer_cv_;
std::condition_variable consumer_cv_;
std::queue<T> queue_;  
std::size_t max_size_;
bool stop_flag_ {false};

public:
 explicit SafeQueue(std::size_t max_size):max_size_(max_size){
assert(max_size > 0);
}
          
 SafeQueue(const SafeQueue&) = delete;
 SafeQueue& operator=(const SafeQueue&) = delete;
 SafeQueue(SafeQueue&&) = delete;
 SafeQueue& operator=(SafeQueue&&) = delete;

  ~SafeQueue(){
 stop();
}
//多生存者 多消费者环境情况
  //push**pop声明          - 版本 A：push**带有开头第一处 `if(stop_flag_) return false;`**   本代码采用A版本
                           //-版本 B：push**删除开头第一处 `if(stop_flag_) return false;`**

//1. **场景①【版本 A，stop 发生在线程拿到锁之前】**
//stop 已经提前执行完毕。线程拿到锁，开头前置判断直接返回，不会进入 wait_for，不休眠。
//2. **场景②【版本 A，stop 发生在线程休眠期间】**
//拿到锁时 stop 为 false；队列满，进入 wait_for 并释放锁、开始休眠；休眠时被 stop 唤醒(其他线程拿到锁调用stop())；wait_for 返回后，靠后置 stop_flag_ 判断返回退出。
//3. **场景③【版本 B，stop 发生在线程拿到锁之前】**
//stop 已经提前执行完毕。拿到锁无前置拦截，直接进入 wait_for；谓词直接成立，**不休眠直接 ok 返回**；依靠后置 stop_flag_ 判断退出。
//4. **场景④【版本 B，stop 发生在线程休眠期间】**
//拿到锁时 stop 为 false；队列满，进入 wait_for 释放锁、休眠；休眠时被 stop 唤醒(其他线程拿到锁调用stop())；wait_for 返回后，靠后置 stop_flag_ 判断返回退出。
//pop同理 但看条件判断

bool push(const T& data,std::chrono::milliseconds timeout=std::chrono::milliseconds(200))  //(左值)
{
    std::unique_lock<std::mutex> lock(mtx_);
      if(stop_flag_) return false;            //前置
                                                 
    if constexpr(Policy == safe_queue_mod::QueueFullPolicy::BLOCK_WAIT){
    bool ok = producer_cv_.wait_for(lock,timeout,[this](){
        return stop_flag_ || queue_.size() < max_size_;    
    });
    if(!ok) return false;
if(stop_flag_) return false;        //后置   当然包括如果休眠被‘停工’唤醒 然后队列还是满的！ 到这也能阻止push
}
   else if constexpr(Policy == safe_queue_mod::QueueFullPolicy::DROP_OLDEST){      //DROP_OLDEST模式不用timeout
       while(queue_.size()>=max_size_){
            queue_.pop();
    }
}

    //如果走到这，就是queue_.size() < max_size_
    queue_.push(data);
    consumer_cv_.notify_one();
    return true;
}

  bool push(T&& data,std::chrono::milliseconds timeout=std::chrono::milliseconds(200)){      //(右值)
  std::unique_lock<std::mutex> lock(mtx_);
       if(stop_flag_){
return false;                       
}        

if constexpr(Policy == safe_queue_mod::QueueFullPolicy::BLOCK_WAIT){
      bool ok = producer_cv_.wait_for(lock,timeout,[this](){    
    return stop_flag_ || queue_.size() < max_size_;      
});

if(!ok)
{
    return false;
  }
if(stop_flag_) return false;                       
}
else if constexpr(Policy == safe_queue_mod::QueueFullPolicy::DROP_OLDEST){         //DROP_OLDEST模式不用timeout

if(queue_.size()>=max_size_){
queue_.pop();
}
}

queue_.push(std::move(data));
consumer_cv_.notify_one();
return true;
} 

bool pop(T& outdata,std::chrono::milliseconds timeout=std::chrono::milliseconds(200)){
std::unique_lock<std::mutex> lock(mtx_);

if(stop_flag_) return false;      
 bool ok=consumer_cv_.wait_for(lock,timeout,[this](){
return stop_flag_||!queue_.empty();               
}
);

if(!ok){
return false;
}

if(stop_flag_){      //    如果休眠被stop()唤醒   到这也不用在意队列是否还有数据 直接退出!  例如push放了数据后直接调用stop(),来不及拿出
return false;              
}

//如果走到这 就只是!queue_.empty()
outdata=std::move(queue_.front());
queue_.pop();
producer_cv_.notify_one();
return true;
}

//移动取出全部元素，原队列清空，支持unique_ptr
std::vector<T> get_queue_all_move(){
    std::vector<T> res;
    std::lock_guard<std::mutex> lock(mtx_);
    res.reserve(queue_.size());
    while(!queue_.empty()){
        res.push_back(std::move(queue_.front()));
        queue_.pop();
    }
    return res;
}

bool empty() const{
std::lock_guard<std::mutex> lock(mtx_);
return queue_.empty();
}

std::size_t queue_size() const{
std::lock_guard<std::mutex> lock(mtx_);
return queue_.size();
}

void stop(){
std::lock_guard<std::mutex> lock(mtx_);
stop_flag_=true;
producer_cv_.notify_all();
consumer_cv_.notify_all();
}

void reset(){
        std::lock_guard<std::mutex> lock(mtx_);
        stop_flag_ = false;
    }

bool is_stopped() const{   //用于测试调用     
std::lock_guard<std::mutex> lock(mtx_);
return stop_flag_;
}

void clear(){
std::lock_guard<std::mutex> lock(mtx_);
while(!queue_.empty()){
    queue_.pop();
}
}
};
}

