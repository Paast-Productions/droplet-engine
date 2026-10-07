#pragma once

#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>

namespace Droplet
{
    /// @brief Universal threadpool class. 
    class ThreadPool
    {
    public:
        /// @return The global instance of the thread pool.
        static ThreadPool &GetInstance();
    
    	ThreadPool(const ThreadPool &other) = delete;
    	ThreadPool &operator=(const ThreadPool &p_other) = delete;
    	ThreadPool(ThreadPool &&p_other) noexcept = delete;
    	ThreadPool &operator=(ThreadPool &&p_other) noexcept = delete;
    
        /// @brief Starts the worker threads. Must be called at engine startup.
        void Initialize();
    
        /// @brief Stops the worker threads. Must be called at engine shutdown.
        void Shutdown();
        
    	/// @brief Push a function that a thread will execute. 
    	/// Everything inside the {} will be executed
    	/// @param p_task Pointer to a function 
    	void PushTask(std::function<void()> p_task);
    
    private:
        ThreadPool() = default;
        ~ThreadPool();
        
    	/// @brief Worker loop that sleeps while idle and execute tasks as they become available.
    	void WorkerLoop();
        
    	std::mutex m_taskMutex;
    	std::condition_variable m_condition; 
    
    	std::queue<std::function<void()>> m_tasks;
    	std::vector<std::thread> m_workerThreads;
    
    	bool m_running = true;
    
    	std::uint32_t m_nrOfThreads = 0;
    };
}

