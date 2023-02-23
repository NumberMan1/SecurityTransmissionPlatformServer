#include "base_shm.h"

// 通过名字打开共享内存
shm::BaseShm::BaseShm(std::string_view name)
    : shm_{boost::interprocess::open_only, name.data(), 
           boost::interprocess::read_write},
      shm_map_{shm_, boost::interprocess::read_write} {
     
}
// 通过名字创建共享内存
shm::BaseShm::BaseShm(std::string_view name, boost::interprocess::offset_t length)
    : shm_{boost::interprocess::create_only, name.data(), 
           boost::interprocess::read_write},
      shm_map_{shm_, boost::interprocess::read_write} {
    shm_.truncate(length);
    std::memset(shm_map_.get_address(), 0, length);
}

