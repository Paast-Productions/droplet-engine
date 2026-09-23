#include "asset/ThreadPool.hpp"

ThreadPool::ThreadPool()
{
	for (uint32_t i = 0; i < 4; i++)
	{
		m_workerThreads.emplace_back(&ThreadPool::WorkerLoop, this);
	}
}

ThreadPool::~ThreadPool()
{
	{
		std::lock_guard<std::mutex> lock(m_taskMutex);
		m_running = false;
	}

	m_condition.notify_all();

	for (std::thread &worker : m_workerThreads)
	{
		if (worker.joinable())
		{
			worker.join();
		}
	}
}
// Comments
void ThreadPool::PushTask(std::function<void()> p_task)
{
	{
		std::lock_guard<std::mutex> lock(m_taskMutex);

		if (!m_running)
		{
			return;
		}

		m_tasks.push(std::move(p_task));
	}

	m_condition.notify_one();
}

void ThreadPool::WorkerLoop()
{
	while (true)
	{
		std::function<void()> task;

		{
			std::unique_lock<std::mutex> lock(m_taskMutex);

			m_condition.wait(lock, [this]()
				{ 
					return !m_tasks.empty() || !m_running; 
				});

			if (!m_running && m_tasks.empty())
			{
				return;
			}

			task = std::move(m_tasks.front());
			m_tasks.pop();

		}
		task();
	}
}