#include <gtest/gtest.h>
#include "memory_pool.hpp"
#include <vector>

using namespace memory_pool_mod;

TEST(MemoryPoolTest, ConstructByBase)
{
    RandomMemoryPool pool(64U, 10U);
    ASSERT_EQ(pool.get_total_blocks(), 10U);
    ASSERT_EQ(pool.get_used_blocks(), 0U);
    ASSERT_EQ(pool.get_free_blocks(), 10U);
    ASSERT_TRUE(pool.is_own_memory());
}

TEST(MemoryPoolTest, ConstructByExternal){
    std::vector<uint8_t> buf(64U * 10U);
    RandomMemoryPool pool(buf.data(), 64U, 10U);
    ASSERT_EQ(pool.get_total_blocks(), 10U);
    ASSERT_EQ(pool.get_used_blocks(), 0U);
    ASSERT_EQ(pool.get_free_blocks(), 10U);
    ASSERT_FALSE(pool.is_own_memory());
}

TEST(MemoryPoolTest, AllocByBlock){
    RandomMemoryPool pool(64U, 10U);
    void* p1=pool.allocate_by_block(2U);
    ASSERT_EQ(pool.get_used_blocks(), 2U);
    ASSERT_EQ(pool.get_free_blocks(), 8U);
    ASSERT_NE(p1, nullptr);
}

TEST(MemoryPoolTest, AllocByBytes){
    RandomMemoryPool pool(64U, 10U);
    void*  p1=pool.allocate_by_bytes(65U);
    ASSERT_EQ(pool.get_used_blocks(), 2U);
    ASSERT_EQ(pool.get_free_blocks(), 8U);
    ASSERT_NE(p1, nullptr);
}

TEST(MemoryPoolTest, AllocByBlockAndBytes){
    RandomMemoryPool pool(64U, 10U);
    void* p1=pool.allocate_by_block(2U);
    void* p2=pool.allocate_by_bytes(68U);
    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);
    ASSERT_EQ(pool.get_used_blocks(), 4U);
    ASSERT_EQ(pool.get_free_blocks(), 6U);
}

TEST(MemoryPoolTest, AllocOverAndZero){
    RandomMemoryPool pool(64U, 10U);
    void* p1=pool.allocate_by_block(0U);
    EXPECT_EQ(p1, nullptr);
    void* p_big=pool.allocate_by_block(20U);
    EXPECT_EQ(p_big, nullptr);
    void* p_byte0=pool.allocate_by_bytes(0U);
    EXPECT_EQ(p_byte0, nullptr);
    void* p_bytes=pool.allocate_by_bytes(650U);
    EXPECT_EQ(p_bytes, nullptr);
}

TEST(MemoryPoolTest, NoFree){
    RandomMemoryPool pool(64U, 10U);
    void* p1=pool.allocate_by_block(3U);
    void* p2=pool.allocate_by_block(4U);
    void* p3=pool.allocate_by_block(3U);
    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);
    ASSERT_NE(p3, nullptr);
    void* p4=pool.allocate_by_block(2U);
    ASSERT_EQ(p4, nullptr);
    ASSERT_EQ(pool.get_free_blocks(), 0U);
}

TEST(MemoryPoolTest, DeallocNormal){
    RandomMemoryPool pool(64U, 10U);
    void* p1=pool.allocate_by_block(3U);
    ASSERT_NE(p1, nullptr);

    bool ret=pool.deallocate(p1);
    ASSERT_TRUE(ret);
    ASSERT_EQ(pool.get_free_blocks(), 10U);
}

TEST(MemoryPoolTest, DoubleFree){
    RandomMemoryPool pool(64U, 10U);
    void* p1=pool.allocate_by_block(3U);
    ASSERT_NE(p1, nullptr);

    ASSERT_TRUE(pool.deallocate(p1));
    ASSERT_FALSE(pool.deallocate(p1));
}

TEST(MemoryPoolTest, BadPtr){
    RandomMemoryPool pool(64U, 10U);
    int dummy=0;
    bool ret=pool.deallocate(&dummy);
    ASSERT_FALSE(ret);

    ASSERT_FALSE(pool.deallocate(nullptr));
}

TEST(MemoryPoolTest, Offset){
    RandomMemoryPool pool(64U, 20U);
    uint8_t* p=static_cast<uint8_t*>(pool.allocate_by_block(2U));

    ASSERT_FALSE(pool.deallocate(p+20));
    ASSERT_TRUE(pool.deallocate(p));
}

TEST(MemoryPoolTest, DeallocAndMergeLeft){
    RandomMemoryPool pool(64U, 20U);
    void* p1=pool.allocate_by_block(2U);
    void* p2=pool.allocate_by_block(5U);
    void* p3=pool.allocate_by_block(13U);
    (void)p3;

    pool.deallocate(p1);
    pool.deallocate(p2);
    ASSERT_EQ(pool.get_used_blocks(), 13U);
    ASSERT_EQ(pool.get_free_blocks(), 7U);
}

TEST(MemoryPoolTest, DeallocAndMergeLeftAndRight){
    RandomMemoryPool pool(64U, 20U);
    void* p1=pool.allocate_by_block(3U);
    void* p2=pool.allocate_by_block(6U);
    void* p3=pool.allocate_by_block(11U);
    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);
    ASSERT_NE(p3, nullptr);
    ASSERT_EQ(pool.get_used_blocks(), 20U);

    pool.deallocate(p1);
    pool.deallocate(p3);
    ASSERT_EQ(pool.get_used_blocks(), 6U);

    pool.deallocate(p2);
    ASSERT_EQ(pool.get_free_blocks(), 20U);
}

TEST(MemoryPoolTest, DeallocAndMergeRight){
    RandomMemoryPool pool(64U, 20U);
    void* p1=pool.allocate_by_block(5U);
    (void)p1;
    void* p2=pool.allocate_by_block(3U);
    void* p3=pool.allocate_by_block(12U);
    ASSERT_EQ(pool.get_used_blocks(), 20U);

    pool.deallocate(p3);
    ASSERT_EQ(pool.get_free_blocks(), 12U);
    pool.deallocate(p2);
    ASSERT_EQ(pool.get_free_blocks(), 15U);
}

TEST(MemoryPoolTest, DeallocNoMergeLeftAndRight){
    RandomMemoryPool pool(64U, 20U);
    void* p1=pool.allocate_by_block(5U);
    void* p2=pool.allocate_by_block(3U);
    (void)p2;
    void* p3=pool.allocate_by_block(4U);
    void* p4=pool.allocate_by_block(6U);
    (void)p4;
    void* p5=pool.allocate_by_block(2U);
    ASSERT_EQ(pool.get_used_blocks(), 20U);

    pool.deallocate(p1);
    pool.deallocate(p5);
    ASSERT_EQ(pool.get_free_blocks(), 7U);
    pool.deallocate(p3);
    ASSERT_EQ(pool.get_free_blocks(), 11U);
}

TEST(MemoryPoolTest, GetTotalBlocks){
    RandomMemoryPool pool(64U, 20U);
    ASSERT_EQ(pool.get_total_blocks(),20U);
}

TEST(MemoryPoolTest, GetUsedBlocks){
    RandomMemoryPool pool(64U, 20U);
    void* p=pool.allocate_by_block(2U);
    (void)p;
    ASSERT_EQ(pool.get_used_blocks(),2U);
}

TEST(MemoryPoolTest, GetFreeBlocks){
    RandomMemoryPool pool(64U, 20U);
    void* p=pool.allocate_by_block(3U);
    (void)p;
    ASSERT_EQ(pool.get_free_blocks(),17U);
}

