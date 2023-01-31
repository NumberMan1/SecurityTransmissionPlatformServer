/*
参考https://github.com/OneLoneCoder/Javidx9/blob/master/PixelGameEngine/BiggerProjects/Networking
进行了一些补充
*/
#pragma once
#include <deque>
#include <mutex>
#include <condition_variable>

namespace mine_net {
    
template<typename T>
class TSQueue {
public:
    TSQueue() = default;
    TSQueue(const TSQueue<T>&) = delete;
    virtual ~TSQueue() { clear(); }

public:
    const T& front() {
        std::scoped_lock lock(muxQueue);
        return deqQueue.front();
    }

    const T& back() {
        std::scoped_lock lock(muxQueue);
        return deqQueue.back();
    }

    T pop_front() {
        std::scoped_lock lock(muxQueue);
        auto t = std::move(deqQueue.front());
        deqQueue.pop_front();
        return t;
    }

    T pop_back() {
        std::scoped_lock lock(muxQueue);
        auto t = std::move(deqQueue.back());
        deqQueue.pop_back();
        return t;
    }

    void push_back(const T& item) {
        std::scoped_lock lock(muxQueue);
        deqQueue.emplace_back(std::move(item));

        std::unique_lock<std::mutex> ul(muxBlocking);
        cvBlocking.notify_one();
    }

    void push_front(const T& item) {
        std::scoped_lock lock(muxQueue);
        deqQueue.emplace_front(std::move(item));

        std::unique_lock<std::mutex> ul(muxBlocking);
        cvBlocking.notify_one();
    }

    bool empty() const {
        std::scoped_lock lock(muxQueue);
        return deqQueue.empty();
    }

    std::size_t size() const {
        std::scoped_lock lock(muxQueue);
        return deqQueue.size();
    }

    void clear() {
        std::scoped_lock lock(muxQueue);
        deqQueue.clear();
    }

    void wait() {
        while (empty()) {
            std::unique_lock<std::mutex> ul(muxBlocking);
            cvBlocking.wait(ul);
        }
    }

protected:
    mutable std::mutex muxQueue;
    mutable std::mutex muxBlocking;
    std::deque<T> deqQueue;
    std::condition_variable cvBlocking;
};
}
