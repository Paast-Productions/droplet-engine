#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <atomic>

#include "Debug/Logger.hpp"
#include "Core/ThreadPool.hpp"

using Droplet::Core::Threads::ThreadPool;
using Droplet::Debug::Logger;

class ThreadPoolLoggerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ThreadPool::GetInstance().Initialize();

        Logger::GetInstance().ClearLog();
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
        log.Log(Droplet::Debug::LogType::Info, "Systemet och trådpoolen är startade!");
        log.Log(Droplet::Debug::LogType::Debug, "Testar loggning via trådpoolen");
        });
	log.WriteToJsonFile();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_TRUE(std::filesystem::exists("logger.json"));
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

        log.Log(Droplet::Debug::LogType::Info, "Alexander är väldigt vacker - Frame: " + std::to_string(frameCounter));
        if (frameCounter % 10 == 0)
        {
			log.WriteToJsonFile();
        }
    }

    EXPECT_EQ(processedFrames, totalFrames);
    EXPECT_TRUE(std::filesystem::exists("logger.json"));
}

TEST_F(ThreadPoolLoggerTest, ClearLogRemovesFile)
{
    Logger &log = Logger::GetInstance();
    ThreadPool &pool = ThreadPool::GetInstance();
    log.Log(Droplet::Debug::LogType::Info, "Tillfälligt loggmeddelande");
    log.WriteToJsonFile();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    ASSERT_TRUE(std::filesystem::exists("logger.json"));

    log.ClearLog();

    EXPECT_FALSE(std::filesystem::exists("logger.json"));
}