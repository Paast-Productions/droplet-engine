#pragma once

#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>

class ThreadPool
{
public:
	ThreadPool();
	~ThreadPool();

	void PushTask(std::function<void()> p_task);

private:
	void WorkerLoop();

private:
	std::mutex m_taskMutex;
	std::condition_variable m_condition; 

	std::queue<std::function<void()>> m_tasks;
	std::vector<std::thread> m_workerThreads;

	bool m_running = true;
};