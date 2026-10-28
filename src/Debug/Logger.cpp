#include "Debug/Logger.hpp"
#include <format>
#include <json/json.hpp>
#include <iostream>

using json = nlohmann::json;

namespace Droplet::Debug
{
    void Log(LogType p_type, const std::string& p_msg, const std::source_location& p_location)
    {
        std::string lineNumber = "The line number: " + std::to_string(p_location.line());

		std::string status{};
		switch (p_type)
		{
		case LogType::Error:   status = "Error"; break;
		case LogType::Warning: status = "Warning"; break;
		case LogType::Info:    status = "Info"; break;
		case LogType::Debug:   status = "Debug"; break;
		}

		auto second = std::chrono::system_clock::now();

		auto local_time = std::chrono::zoned_time{ std::chrono::current_zone(), second };
		std::string timeStr = std::format("{:%Y-%m-%d %H:%M:%S}", local_time);

		std::cout << "Status: " << status << " | Message: "
			<< p_msg << " | Function: " << p_location.function_name() <<
			" | File: " << p_location.file_name() << " | Line: "
			<< lineNumber << std::endl;
		
    }
}