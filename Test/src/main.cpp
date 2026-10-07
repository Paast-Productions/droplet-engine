#include <gtest/gtest.h>
#include <Debug/Logger.hpp>

using namespace Droplet::Debug;
int main(int argc, char **argv)
{
    Logger log;
    log.ClearLog();
    log.Log(Logger::LogType::Error, "hello world");

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
