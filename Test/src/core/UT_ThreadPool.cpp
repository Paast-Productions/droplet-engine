#include "core/ThreadPool.hpp"
#include <gtest/gtest.h>

#include <atomic>
#include <future>
#include <chrono>

using namespace Droplet;

class ThreadPoolTest : public ::testing::Test
{
protected:
    void SetUp() override
    {

        ThreadPool::GetInstance().Initialize();
    }

    void TearDown() override
    {
        ThreadPool::GetInstance().Shutdown();
    }
};

TEST_F(ThreadPoolTest, ExecuteSingleTask)
{
    // Promise allows a background thread to signal the main thread that it's done
    std::promise<void> taskPromise;
    std::future<void> taskFuture = taskPromise.get_future();

    ThreadPool::GetInstance().PushTask([&taskPromise]() 
    {
        taskPromise.set_value(); // Signal that the task ran
    });

    // Wait up to 2 seconds for the task to finish
    std::future_status status = taskFuture.wait_for(std::chrono::seconds(2));
    EXPECT_EQ(status, std::future_status::ready) << "Task did not execute in time (Timeout).";
}

TEST_F(ThreadPoolTest, ExecutesOnWorkerThread)
{
    std::promise<std::thread::id> idPromise;
    std::future<std::thread::id> idFuture = idPromise.get_future();
    std::thread::id mainThreadId = std::this_thread::get_id();

    ThreadPool::GetInstance().PushTask([&idPromise]() 
    {
        // Send the worker's thread ID back to the main thread
        idPromise.set_value(std::this_thread::get_id());
    });

    ASSERT_EQ(idFuture.wait_for(std::chrono::seconds(2)), std::future_status::ready);
    
    std::thread::id workerThreadId = idFuture.get();
    
    // The worker thread ID should not be the main thread ID
    EXPECT_NE(workerThreadId, mainThreadId);
}

TEST_F(ThreadPoolTest, ExecuteMultipleTasks)
{
    constexpr int numTasks = 1000;
    std::atomic<int> completedTasks{0};

    // Spam the thread pool with tasks
    for (int i = 0; i < numTasks; ++i)
    {
        ThreadPool::GetInstance().PushTask([&completedTasks]() 
        {
            completedTasks.fetch_add(1, std::memory_order_relaxed);
        });
    }

    // Wait for all tasks to complete (5-second timeout)
    auto startTime = std::chrono::steady_clock::now();
    while (completedTasks.load() < numTasks)
    {
        if (std::chrono::steady_clock::now() - startTime > std::chrono::seconds(5))
        {
            FAIL() << "Timeout waiting for all tasks to complete. Only completed: " << completedTasks.load();
        }
        
        std::this_thread::yield(); 
    }

    EXPECT_EQ(completedTasks.load(), numTasks);
}