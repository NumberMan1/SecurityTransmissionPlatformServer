#ifndef BASE_SHM_H
#define BASE_SHM_H

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <string>

namespace shm {

class BaseShm {
private:
    boost::interprocess::shared_memory_object shm_;
    boost::interprocess::mapped_region shm_map_;
public:
    // 通过名字打开共享内存
    explicit BaseShm(std::string_view name);
    // 通过名字创建共享内存
    explicit BaseShm(std::string_view name, boost::interprocess::offset_t length);
    virtual ~BaseShm() = default;
    // 扩建
    inline void Truncate(boost::interprocess::offset_t length) {
        shm_.truncate(shm_map_.get_size() + length);
    }
    // 销毁共享内存
    inline bool DeleteShm() {
        return boost::interprocess::shared_memory_object::remove(shm_.get_name());
    }
    inline std::string_view GetName() const noexcept {
        return shm_.get_name();
    }
    inline void* GetAddress() const noexcept {
        return shm_map_.get_address();
    }
    inline std::size_t Size() const noexcept {
        return shm_map_.get_size();
    }
};

} // namsepace shm

#endif