#pragma once

#include <mutex>
#include <queue>

namespace Droplet
{
    template<typename T>
    class ThreadSafeQueue
    {
    public:
        ThreadSafeQueue() = default;
        ~ThreadSafeQueue() = default;
        
        ThreadSafeQueue(const ThreadSafeQueue&) = delete;
        ThreadSafeQueue operator=(const ThreadSafeQueue&) = delete;
        
        void Push(T p_item)
        {
            std::scoped_lock lock(m_mutex);
            m_queue.push(std::move(p_item));
        }
        
        [[nodiscard]] bool Pop(T &p_item)
        {
            std::scoped_lock lock(m_mutex);
            
            if (m_queue.empty())
            {
                return false;
            }
            
            p_item = m_queue.front();
            m_queue.pop();
            return true;
        }
        
        [[nodiscard]] bool IsEmpty() const
        {
            std::lock<std::mutex> lock(m_mutex);
            return m_queue.empty();
        }
        
    private:
        mutable std::mutex m_mutex; // Mutable so that it can be locked inside const functions
        std::queue<T> m_queue;
    };
}