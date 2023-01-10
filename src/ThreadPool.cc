#include "ThreadPool.hpp"

mine_thread::ThreadPool::ThreadPool(size_t threads)
    :lock_()
    ,threads_()
    ,tasks_()
    ,cond_()
    ,running_(true)
{
    for (size_t i = 0; i < threads; ++i)
    {
        this->threads_.emplace_back(std::bind(&ThreadPool::ThreadFunc,this));   
    }
}

void mine_thread::ThreadPool::ThreadFunc()
{
    while (this->running_)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(this->lock_);
            while (this->tasks_.empty() && running_)
            {
                this->cond_.wait(lock);
            }
            if(!running_)
            {
                return;
            }
            task = std::move(this->tasks_.back());
            this->tasks_.pop_back();
        }
        if(task)
        {
            task();
        }
    }
}

void mine_thread::ThreadPool::Stop() noexcept
{
    if(this->running_)
    {
        this->running_ = false;
        this->cond_.notify_all();
        for (auto begin = this->threads_.begin(),end = this->threads_.end(); begin != end; ++begin)
        {
            begin->join();
        }
    }
}