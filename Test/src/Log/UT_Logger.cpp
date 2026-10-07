#include <gtest/gtest.h>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
#include <filesystem>
#include "Debug/Logger.hpp"
using Droplet::Debug::Logger;

TEST(ThreadTest, ThreadRuns)
{
    Logger log;

    log.Log(Logger::LogType::Info, "Starting ThreadRuns test");

    bool executed = false;

    std::thread worker([&]()
        {
            log.Log(Logger::LogType::Info, "Worker thread started");

            executed = true;

            log.Log(Logger::LogType::Info, "Worker thread finished");
        });

    worker.join();

    EXPECT_TRUE(executed);

    log.Log(Logger::LogType::Info, "ThreadRuns test finished");
    log.ClearLog();
}


TEST(ThreadTest, ThreadChangesAtomicValue)
{
    Logger log;

    log.Log(Logger::LogType::Info,
        "Starting ThreadChangesAtomicValue test");

    std::atomic<int> value{ 0 };

    std::thread worker([&]()
        {
            log.Log(Logger::LogType::Info,
                "Worker changing atomic value");

            value.store(42);

            log.Log(Logger::LogType::Info,
                "Worker stored value 42");
        });

    worker.join();

    EXPECT_EQ(value.load(), 42);

    log.Log(Logger::LogType::Info,
        "ThreadChangesAtomicValue test finished");
    log.ClearLog();
}


TEST(ThreadTest, MultipleThreadsIncrementValue)
{
    Logger log;

    log.Log(Logger::LogType::Info,
        "Starting MultipleThreadsIncrementValue test");

    constexpr int threadCount = 10;
    constexpr int incrementsPerThread = 42;

    std::atomic<int> counter{ 0 };

    std::vector<std::thread> threads;

    for (int i = 0; i < threadCount; ++i)
    {
        threads.emplace_back([&counter, &log, i]()
            {
                log.Log(Logger::LogType::Info,
                    "Worker thread started");

                for (int j = 0; j < incrementsPerThread; ++j)
                {
                    counter.fetch_add(1);
                }

                log.Log(Logger::LogType::Info,
                    "Worker thread finished");
            });
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    const int expected = threadCount * incrementsPerThread;

    EXPECT_EQ(counter.load(), expected);

    log.Log(Logger::LogType::Info,
        "MultipleThreadsIncrementValue test finished");
    log.ClearLog();
}


TEST(ThreadTest, MutexProtectsSharedData)
{
    Logger log;

    log.Log(Logger::LogType::Info,
        "Starting MutexProtectsSharedData test");

    constexpr int threadCount = 10;
    constexpr int incrementsPerThread = 1000;

    int counter = 0;

    std::mutex mutex;

    std::vector<std::thread> threads;

    for (int i = 0; i < threadCount; ++i)
    {
        threads.emplace_back([&counter, &mutex, &log]()
            {
                log.Log(Logger::LogType::Info,
                    "Thread waiting for mutex");

                for (int j = 0; j < incrementsPerThread; ++j)
                {
                    std::lock_guard<std::mutex> lock(mutex);

                    ++counter;
                }

                log.Log(Logger::LogType::Info,
                    "Thread finished");
            });
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    const int expected = threadCount * incrementsPerThread;

    EXPECT_EQ(counter, expected);

    log.Log(Logger::LogType::Info,
        "MutexProtectsSharedData test finished");
    log.ClearLog();
}


TEST(ThreadTest, ThreadsCanRunInParallel)
{
    Logger log;

    log.Log(Logger::LogType::Info,
        "Starting ThreadsCanRunInParallel test");

    constexpr int threadCount = 4;

    std::atomic<int> completedThreads{ 0 };

    std::vector<std::thread> threads;

    for (int i = 0; i < threadCount; ++i)
    {
        threads.emplace_back([&completedThreads, &log]()
            {
                log.Log(Logger::LogType::Info,
                    "Thread executing");

                std::this_thread::yield();

                ++completedThreads;

                log.Log(Logger::LogType::Info,
                    "Thread completed");
            });
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    EXPECT_EQ(completedThreads.load(), threadCount);

    log.Log(Logger::LogType::Info,
        "ThreadsCanRunInParallel test finished");
    log.ClearLog();
}

TEST(ThreadTest, TestingAllEnum)
{
    EXPECT_NO_THROW({
            Logger log;
            log.Log(Logger::LogType::Info, "info");
            log.Log(Logger::LogType::Debug, "debug");
            log.Log(Logger::LogType::Error, "error");
            log.Log(Logger::LogType::Warning, "warning");
        });
}

TEST(ThreadTest, ClearLog)
{
    Logger log;
    log.Log(Logger::LogType::Info, "See if the clearLog works");

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    EXPECT_TRUE(std::filesystem::exists("logger.json"));
    log.ClearLog();
    EXPECT_FALSE(std::filesystem::exists("logger.json"));    
}