#ifndef SECKEY_SHM_HPP
#define SECKEY_SHM_HPP

#include "base_shm.h"

namespace shm {

template<typename T, std::size_t max_node_num>
class SeckeyShm : public BaseShm {
private:
    std::size_t num_node_{0};
public:
    // 通过名字打开或创建共享内存
    explicit SeckeyShm(std::string_view name)
        : BaseShm(name, sizeof(std::array<T, max_node_num>)) {
        
    }
    virtual ~SeckeyShm() = default;
    inline std::size_t MaxNodeNum() const noexcept {
        return max_node_num;
    }
    inline void Reset() noexcept {
        num_node_ = 0;
    }
    // T需要支持==
    T* Find(const T target) const {
        // T* temp = static_cast<T*>(GetAddress());
        // std::size_t len = Size();
        // for (T* end = static_cast<T*>(GetAddress()) + len;
        //      temp != end; ++temp) {
        //     if (*temp == target) {
        //         return temp;
        //     }
        // }
        std::array<T, max_node_num> *arr = 
            static_cast<std::array<T, max_node_num>*>(GetAddress());
        for (std::size_t i = 0; i != num_node_; ++i) {
            if (target == (*arr)[i]) {
                return &(*arr)[i];
            }
        }
        return nullptr;
    }
    template<typename Handle>
    T* Find(const T target, Handle h) const {
        // T* temp = static_cast<T*>(GetAddress());
        // std::size_t len = Size();
        // for (T* end = static_cast<T*>(GetAddress()) + len;
        //      temp != end; ++temp) {
        //     if (h(*temp, target)) {
        //         return temp;
        //     }
        // }
        std::array<T, max_node_num> *arr = 
            static_cast<std::array<T, max_node_num>*>(GetAddress());
        for (std::size_t i = 0; i != num_node_; ++i) {
            if (h(target, (*arr)[i])) {
                return &(*arr)[i];
            }
        }
        return nullptr;
    }
    bool Write(const T target) {
        // std::size_t len_before = Size();
        // Truncate(sizeof(T));
        // T* temp = GetAddress();
        // temp += len_before;
        // *temp = target;
        std::array<T, max_node_num> *arr = 
            static_cast<std::array<T, max_node_num>*>(GetAddress());
        if (num_node_ != max_node_num) {
            (*arr)[num_node_++] = target;
        } else {
            return false;
        }
        return true;
    }
};

} // namespace shm


#endif // SECKEY_SHM_HPP