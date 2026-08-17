#include"memory_pool.hpp"
#include <cassert>

namespace memory_pool_mod{

void RandomMemoryPool::destory_free_list(){
  FreeNode* cur=free_list_;
while(cur!=nullptr){
  FreeNode* tmp=cur;
cur = cur->next;
delete tmp;
}
free_list_=nullptr;
}

void RandomMemoryPool::merge_free_segment(std::size_t start_idx, std::size_t block_cnt){
        std::size_t end_idx=start_idx +block_cnt -1U;
FreeNode** pp=&free_list_;
FreeNode* prev_node=nullptr;
FreeNode* cur_node=free_list_;

while(cur_node != nullptr && cur_node->start_idx < start_idx){
prev_node=cur_node;
pp =&cur_node->next;
cur_node=cur_node->next;
}

bool merge_left = false;
if(prev_node !=nullptr){
std::size_t left_end= prev_node->start_idx +prev_node->block_cnt -1U;
if(left_end +1U == start_idx){
prev_node->block_cnt += block_cnt;
start_idx = prev_node-> start_idx;
block_cnt =prev_node->block_cnt;
end_idx =prev_node->start_idx +prev_node->block_cnt -1U;
merge_left = true;
}
}

bool merge_right = false;
if(cur_node != nullptr){
std::size_t right_start = cur_node->start_idx;
if(end_idx + 1U == right_start){

if(merge_left){
prev_node->block_cnt += cur_node->block_cnt;
FreeNode* del =cur_node;
cur_node= cur_node-> next;
prev_node->next= cur_node;
delete del;
merge_right = true;
}
else {
FreeNode* new_node= new FreeNode(start_idx,block_cnt + cur_node->block_cnt);
new_node->next =cur_node->next;
*pp=new_node;
delete cur_node;
return;
}
}
}

if(!merge_left&&!merge_right){
FreeNode* new_node= new FreeNode(start_idx,block_cnt);
new_node->next=cur_node;
*pp=new_node;
 }
}

RandomMemoryPool::RandomMemoryPool(std::size_t block_size, std::size_t total_blocks)
    : block_size_(block_size)
    , total_blocks_(total_blocks)
    , base_ptr_(nullptr)
    , own_memory_(true)
    , mtx_()
    , free_list_(nullptr)
    , alloc_record_()
{
assert(block_size>0U);
assert(total_blocks>0U);

std::size_t total_byte=block_size * total_blocks;
base_ptr_= new uint8_t[total_byte];
free_list_=new FreeNode(0U, total_blocks);
}

RandomMemoryPool::RandomMemoryPool(uint8_t* base_buf,std::size_t block_size, std::size_t total_blocks)
    : block_size_(block_size)
    , total_blocks_(total_blocks)
    , base_ptr_(base_buf)
    , own_memory_(false)
    , mtx_()
    , free_list_(nullptr)
    , alloc_record_()
{
assert(block_size>0U);
assert(total_blocks>0U);
assert(base_buf != nullptr);
free_list_ = new FreeNode(0U, total_blocks);
}

RandomMemoryPool::~RandomMemoryPool(){
 destory_free_list();
if(own_memory_ == true&&base_ptr_!=nullptr){
delete[] base_ptr_;
}
base_ptr_=nullptr;
}

void* RandomMemoryPool::allocate_by_block(std::size_t blocks_n){
  std::lock_guard<std::mutex> lock(mtx_);

if(blocks_n ==0U || blocks_n > total_blocks_){
return nullptr;
}

FreeNode** pp=&free_list_;
FreeNode* cur=free_list_;

while(cur!=nullptr){
if(cur->block_cnt >= blocks_n){
  std::size_t alloc_start_idx=cur->start_idx;
if(cur->block_cnt == blocks_n){
  *pp=cur->next;
delete cur;
}
else{
  cur->start_idx += blocks_n;
  cur->block_cnt -= blocks_n;
}
uint8_t* ret =base_ptr_ + alloc_start_idx*block_size_;
alloc_record_[ret] = blocks_n;
return static_cast<void*>(ret);
}
pp= &cur->next;
cur=cur->next;
}
return nullptr;
}

void* RandomMemoryPool::allocate_by_bytes(std::size_t need_bytes){
if(need_bytes== 0U){
return nullptr;
}

std::size_t blocks_n =(need_bytes + block_size_ -1U) / block_size_;
return allocate_by_block(blocks_n);
}

bool RandomMemoryPool::deallocate(void* p){
std::lock_guard<std::mutex> lock(mtx_);

std::size_t idx= ptr_to_index_inner(p, base_ptr_, block_size_, total_blocks_);
if(idx >= total_blocks_) return false;

auto it=alloc_record_.find(p);
if(it==alloc_record_.end()){
return false;
}

std::size_t n =it->second;
alloc_record_.erase(it);

merge_free_segment(idx,n);
return true;
}

std::size_t RandomMemoryPool::ptr_to_index_inner(const void* p, const uint8_t* base,
std::size_t block_size, std::size_t total_blk)
{
const uint8_t* ptr=static_cast<const uint8_t*>(p);
if(ptr<base){
return total_blk;
}

std::ptrdiff_t offset = ptr -base;
if(offset < 0 || static_cast<std::size_t>(offset) >= total_blk * block_size){
return total_blk;
}
return static_cast<std::size_t>(offset)/block_size;
}

void RandomMemoryPool::reset(){
std::lock_guard<std::mutex> lock(mtx_);

destory_free_list();
free_list_ = new FreeNode(0U, total_blocks_);
alloc_record_.clear();
}

std::size_t RandomMemoryPool:: get_total_blocks() const{
std::lock_guard<std::mutex> lock(mtx_);
return total_blocks_;
}

std::size_t RandomMemoryPool::get_used_blocks() const{
std::lock_guard<std::mutex> lock(mtx_);
std::size_t free=0U;
FreeNode* cur=free_list_;
while(cur != nullptr){
free+=cur->block_cnt;
cur=cur->next;
}
return total_blocks_ - free;
}

std::size_t RandomMemoryPool::get_free_blocks() const{
std::lock_guard<std::mutex> lock(mtx_);
std::size_t free=0U;
FreeNode* cur=free_list_;
while(cur != nullptr){
free+=cur->block_cnt;
cur=cur->next;
}
return free;
}

bool RandomMemoryPool::is_own_memory() const{
std::lock_guard<std::mutex> lock(mtx_);
return own_memory_;
}

}

