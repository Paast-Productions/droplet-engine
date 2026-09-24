#pragma once

#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>

/// @brief Universal threadpool class. 
class ThreadPool
{
public:
	/// @brief Creates threads that will execute the worker loop.
	ThreadPool();
	/// @brief Stops the threadpool and worker loop and join all threads.
	~ThreadPool();

	ThreadPool(const ThreadPool &other) = delete;
	ThreadPool &operator=(const ThreadPool &p_other) = delete;
	ThreadPool(ThreadPool &&p_other) noexcept = delete;
	ThreadPool &operator=(ThreadPool &&p_other) noexcept = delete;


	/// @brief Push a function that a thread will execute. 
	/// Everything inside the {} will be executed
	/// @param p_task Pointer to a function 
	void PushTask(std::function<void()> p_task);

private:
	/// @brief Worker loop that sleeps while idle and execute tasks as they become available.
	void WorkerLoop();

private:
	std::mutex m_taskMutex;
	std::condition_variable m_condition; 

	std::queue<std::function<void()>> m_tasks;
	std::vector<std::thread> m_workerThreads;

	bool m_running = true;

	int m_nrOfThreads = 0;
};