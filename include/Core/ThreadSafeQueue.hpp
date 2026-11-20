#pragma once

#include <mutex>
#include <queue>

namespace Droplet
{
    /// @brief Thread-safe queue using a mutex lock.
    template<typename T>
    class ThreadSafeQueue
    {
    public:
        ThreadSafeQueue() = default;
        ~ThreadSafeQueue() = default;
        
        ThreadSafeQueue(const ThreadSafeQueue&) = delete;
        ThreadSafeQueue operator=(const ThreadSafeQueue&) = delete;

        /// @brief Pushes an item into the queue.
        /// @param p_item The item to be pushed.
        void Push(T p_item)
        {
            std::scoped_lock lock(m_mutex);
            m_queue.push(std::move(p_item));
        }

        /// @brief Attempts to pop an item from the queue.
        /// @param p_item The item to be populated.
        /// @return True if an item was successfully popped from the queue. False if the queue was empty.
        [[nodiscard]] bool Pop(T &p_item)
        {
            std::scoped_lock lock(m_mutex);
            
            if (m_queue.empty())
            {
                return false;
            }
            
            p_item = std::move(m_queue.front());
            m_queue.pop();
            return true;
        }
        
        
        /// @brief Checks if the queue is empty.
        /// @return True if the queue is empty, otherwise false.
        [[nodiscard]] bool IsEmpty() const
        {
            std::scoped_lock lock(m_mutex);
            return m_queue.empty();
        }
        
    private:
        mutable std::mutex m_mutex; // Mutable so that it can be locked inside const functions
        std::queue<T> m_queue;
    };
}