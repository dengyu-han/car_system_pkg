#pragma once
#include<mutex>
#include<cstddef>
#include<cstdint>
#include <unordered_map>

namespace memory_pool_mod{

struct FreeNode{
std::size_t start_idx;
std::size_t block_cnt; 
FreeNode* next;

FreeNode(std::size_t s,std::size_t cnt):start_idx(s),block_cnt(cnt),next(nullptr)
{}
};

class RandomMemoryPool{
private:
    // =====需注意此处声明顺序，和构造初始化顺序匹配，消除-Wreorder警告=====
    std::size_t block_size_{0U};
    std::size_t total_blocks_{0U};
    uint8_t* base_ptr_{nullptr};
    bool own_memory_{false};

    mutable std::mutex mtx_;
    FreeNode* free_list_{nullptr};
    std::unordered_map<void*,std::size_t>alloc_record_;

private:
void destory_free_list();
void merge_free_segment(std::size_t start_idx,std::size_t block_cnt);
static std::size_t ptr_to_index_inner(const void* p, const uint8_t* base, std::size_t block_size, std::size_t total_block);

public:
explicit RandomMemoryPool(std::size_t block_size, std::size_t total_block);
RandomMemoryPool(uint8_t* base_buf, std::size_t block_size, std::size_t total_blocks);
~RandomMemoryPool();

RandomMemoryPool(const RandomMemoryPool&)=delete;
RandomMemoryPool& operator=(const RandomMemoryPool&)=delete;

void* allocate_by_block(std::size_t block_n);
void* allocate_by_bytes(std::size_t need_bytes);
bool deallocate(void* p);
void reset();

std::size_t get_total_blocks()const;
std::size_t get_used_blocks() const;
std::size_t get_free_blocks() const;
bool is_own_memory() const;
};

}


