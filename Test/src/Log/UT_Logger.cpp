#include <gtest/gtest.h>
#include "Debug/Logger.hpp"

#include <iostream>
#include <source_location>
#include <string>

namespace Droplet::Debug
{
    class LoggerTest : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            testing::internal::CaptureStdout();
        }

        std::string GetOutput()
        {
            return testing::internal::GetCapturedStdout();
        }
    };

    TEST_F(LoggerTest, LogsErrorMessage)
    {
        Log(LogType::Error, "Something went wrong");

        const std::string output = GetOutput();

        EXPECT_NE(output.find("Status: Error"), std::string::npos);
        EXPECT_NE(output.find("Message: Something went wrong"), std::string::npos);
    }

    TEST_F(LoggerTest, LogsWarningMessage)
    {
        Log(LogType::Warning, "This is a warning");

        const std::string output = GetOutput();

        EXPECT_NE(output.find("Status: Warning"), std::string::npos);
        EXPECT_NE(output.find("Message: This is a warning"), std::string::npos);
    }

    TEST_F(LoggerTest, LogsInfoMessage)
    {
        Log(LogType::Info, "Application started");

        const std::string output = GetOutput();

        EXPECT_NE(output.find("Status: Info"), std::string::npos);
        EXPECT_NE(output.find("Message: Application started"), std::string::npos);
    }

    TEST_F(LoggerTest, LogsDebugMessage)
    {
        Log(LogType::Debug, "Debugging application");

        const std::string output = GetOutput();

        EXPECT_NE(output.find("Status: Debug"), std::string::npos);
        EXPECT_NE(output.find("Message: Debugging application"), std::string::npos);
    }

    TEST_F(LoggerTest, LogsFunctionName)
    {
        Log(LogType::Info, "Testing function name");

        const std::string output = GetOutput();

        EXPECT_NE(
            output.find("LoggerTest_LogsFunctionName_Test"),
            std::string::npos
        );
    }

    TEST_F(LoggerTest, LogsFileName)
    {
        Log(LogType::Info, "Testing file name");

        const std::string output = GetOutput();

        EXPECT_NE(output.find(__FILE__), std::string::npos);
    }

    TEST_F(LoggerTest, LogsLineNumber)
    {
        const auto location = std::source_location::current();

        Log(LogType::Info, "Testing line number", location);

        const std::string output = GetOutput();

        const std::string expected =
            "The line number: " + std::to_string(location.line());

        EXPECT_NE(output.find(expected), std::string::npos);
    }

    TEST_F(LoggerTest, LogsEmptyMessage)
    {
        Log(LogType::Info, "");

        const std::string output = GetOutput();

        EXPECT_NE(output.find("Status: Info"), std::string::npos);
        EXPECT_NE(output.find("Message: "), std::string::npos);
    }
}