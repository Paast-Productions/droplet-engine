#include <gtest/gtest.h>
#include <Debug/Logger.hpp>
#include <Core/ThreadPool.hpp> 

using Droplet::ThreadPool;
using Droplet::Debug::Logger ;
//int main(int argc, char **argv)
//{
//    ThreadPool &pool = ThreadPool::GetInstance();
//    Logger &log = Logger::GetInstance();
//    pool.Initialize();
//    log.ClearLog();
//    int frameCounter = 0;
//    while (true) // Example condition, replace with actual frame counter logic
//    {
//		frameCounter++;
//		if (frameCounter % 10 == 0)
//		{   
//			pool.PushTask([&log]() {
//                log.ProcessQueue();
//            });
//		}
//        log.Log(Logger::LogType::Info, "Alexander är väldigt vacker");
//    }
//    ::testing::InitGoogleTest(&argc, argv);
//    return RUN_ALL_TESTS();
//}

#include <iostream>

using Droplet::ThreadPool;
using Droplet::Debug::Logger;

int main(int argc, char **argv)
{
    //ThreadPool &pool = ThreadPool::GetInstance();
    //pool.Initialize();

    //Logger &log = Logger::GetInstance();

    //pool.PushTask([&log]() {
    //    log.ProcessQueue();
    //    });

    //log.Log(Logger::LogType::Info, "Systemet och trådpoolen är startade!");
    //log.Log(Logger::LogType::Debug, "Testar loggning via trådpoolen");

    //pool.PushTask([]() {
    //    std::cout << "Kör ett fristående jobb i trådpoolen!\n";
    //    });

    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();

    //pool.Shutdown();

    return result;
}
