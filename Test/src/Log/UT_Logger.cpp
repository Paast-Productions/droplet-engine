#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <atomic>

#include "Debug/Logger.hpp"
#include "Core/ThreadPool.hpp"

using Droplet::ThreadPool;
using Droplet::Debug::Logger;

class ThreadPoolLoggerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ThreadPool::GetInstance().Initialize();

        Logger::GetInstance().ClearLog();

        //ThreadPool::GetInstance().PushTask([]() {
        //    Logger::GetInstance().ProcessQueue();
        //    });
    }

    void TearDown() override
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        ThreadPool::GetInstance().Shutdown();
    }
};


TEST_F(ThreadPoolLoggerTest, SystemStartupLogging)
{
    Logger &log = Logger::GetInstance();
	ThreadPool &pool = ThreadPool::GetInstance();
    EXPECT_NO_THROW({
        log.Log(Logger::LogType::Info, "Systemet och trådpoolen är startade!");
        log.Log(Logger::LogType::Debug, "Testar loggning via trådpoolen");
        });
    pool.PushTask([&log]() {
        log.ProcessQueue();
        });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_TRUE(std::filesystem::exists("logger.json"));
}

TEST_F(ThreadPoolLoggerTest, StandaloneTaskExecution)
{
    Logger &log = Logger::GetInstance();
    ThreadPool &pool = ThreadPool::GetInstance();

    std::atomic<bool> taskExecuted{ false };

    pool.PushTask([&log, &taskExecuted]() {
        log.Log(Logger::LogType::Info, "Startar fristående jobb");

        taskExecuted.store(true);

        log.Log(Logger::LogType::Info, "Fristående jobb slutfört");
        });

    int maxWaitMs = 500;
    while (!taskExecuted.load() && maxWaitMs > 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        maxWaitMs -= 10;
    }

    EXPECT_TRUE(taskExecuted.load());
}

TEST_F(ThreadPoolLoggerTest, FrameLoopSimulation)
{
    Logger &log = Logger::GetInstance();
    ThreadPool &pool = ThreadPool::GetInstance();
    constexpr int totalFrames = 50;
    int processedFrames = 0;

    for (int frameCounter = 1; frameCounter <= totalFrames; ++frameCounter)
    {
        processedFrames++;

        log.Log(Logger::LogType::Info, "Alexander är väldigt vacker - Frame: " + std::to_string(frameCounter));
        if (frameCounter % 10 == 0)
        {
            pool.PushTask([frameCounter, &log]() {
                log.ProcessQueue();
                });
        }
    }

    EXPECT_EQ(processedFrames, totalFrames);
    EXPECT_TRUE(std::filesystem::exists("logger.json"));
}

TEST_F(ThreadPoolLoggerTest, ClearLogRemovesFile)
{
    Logger &log = Logger::GetInstance();
    ThreadPool &pool = ThreadPool::GetInstance();
    log.Log(Logger::LogType::Info, "Tillfälligt loggmeddelande");
    pool.PushTask([&log]() {
        log.ProcessQueue();
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    ASSERT_TRUE(std::filesystem::exists("logger.json"));

    log.ClearLog();

    EXPECT_FALSE(std::filesystem::exists("logger.json"));
}