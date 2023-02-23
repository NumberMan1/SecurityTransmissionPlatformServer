#ifndef SECKEY_SHM_HPP
#define SECKEY_SHM_HPP

#include "base_shm.h"

namespace shm {

template<typename T>
class SeckeyShm : public BaseShm {
private:

public:
    // 通过名字打开共享内存
    explicit SeckeyShm(std::string_view name)
        : BaseShm(name) {

    }
    // 通过名字创建共享内存
    explicit SeckeyShm(std::string_view name, std::int16_t max_node)
        : BaseShm(name, max_node * sizeof(T)) {
        
    }
    virtual ~SeckeyShm() = default;
    // T需要支持==
    T* Find(const T target) const {
        T* temp = static_cast<T*>(GetAddress());
        std::size_t len = Size();
        for (T* end = static_cast<T*>(GetAddress()) + len;
             temp != end; ++temp) {
            if (*temp == target) {
                return temp;
            }
        }
        return nullptr;
    }
    template<typename Handle>
    T* Find(const T target, Handle h) const {
        T* temp = static_cast<T*>(GetAddress());
        std::size_t len = Size();
        for (T* end = static_cast<T*>(GetAddress()) + len;
             temp != end; ++temp) {
            if (h(*temp, target)) {
                return temp;
            }
        }
        return nullptr;
    }
    void Write(const T target) {
        std::size_t len_before = Size();
        Truncate(sizeof(T));
        T* temp = GetAddress();
        temp += len_before;
        *temp = target;
    }
};

} // namespace shm


#endif // SECKEY_SHM_HPP