#include "Debug/Logger.hpp"
#include <format>
#include <json/json.hpp>
#include <iostream>

using json = nlohmann::json;

namespace Droplet::Debug
{
    void Log(LogType p_type, const std::string& p_msg, const std::source_location& p_location)
    {
        std::string status{};
        switch (p_type)
        {
        case LogType::Error:   status = "Error"; break;
        case LogType::Warning: status = "Warning"; break;
        case LogType::Info:    status = "Info"; break;
        case LogType::Debug:   status = "Debug"; break;
        }

        std::string lineNumber = "The line number: " + std::to_string(p_location.line());

        LogEntry logEntry{
            std::chrono::system_clock::now(),
            p_msg,
            status,
            std::this_thread::get_id(),
            p_location.function_name(),
            p_location.file_name(),
            lineNumber
        };

        auto second = std::chrono::time_point_cast<std::chrono::seconds>(logEntry.timestamp);

        auto local_time = std::chrono::zoned_time{ std::chrono::current_zone(), second };
        std::string timeStr = std::format("{:%Y-%m-%d %H:%M:%S}", local_time);
        std::string threadStr = std::format("{}", logEntry.threadId);

		std::cout << "Status: " << logEntry.status << " | Message: "
    	<< logEntry.msg << " | Function: " << logEntry.function <<
            " | File: " << logEntry.path << " | Line: "
    	<< logEntry.lineNumber << std::endl;
    }
}