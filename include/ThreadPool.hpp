#ifndef _THREADPOOL_HPP
#define _THREADPOOL_HPP

#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <type_traits>

namespace mine_thread {

class ThreadPool
{
private:
    using Self = ThreadPool;

    std::mutex lock_;
    std::vector<std::thread> threads_;
    std::vector<std::function<void()>> tasks_;
    std::condition_variable cond_;
    bool running_;

    void ThreadFunc();
public:

    explicit ThreadPool(size_t threads);

    ThreadPool(const Self &other) = delete;

    ThreadPool(Self &&other) noexcept = delete;

    Self &operator=(const Self &other) = delete;

    Self &operator=(Self &&other) noexcept = delete;

    ~ThreadPool() noexcept
    {
        this->Stop();
    }

    template<typename _Fn,typename ..._Args,typename _Check = decltype(std::bind(std::declval<_Fn>(),std::declval<_Args>()...))>
    void Put(_Fn &&fn,_Args &&...args)
    {
        {
            std::unique_lock<std::mutex> lock(this->lock_);
            this->tasks_.emplace_back(std::bind(std::forward<_Fn>(fn),std::forward<_Args>(args)...));
            this->cond_.notify_one();
        }
    }

    void Stop() noexcept;
};

} // namespace mine_thread
#endif // _THREADPOOL_HPP