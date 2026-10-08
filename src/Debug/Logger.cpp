#include "Debug/Logger.hpp"

#include <format>
#include <fstream>
#include <thread>

#include <json/json.hpp>
#include <iostream>

using json = nlohmann::json;
using namespace Droplet::Debug;
Logger::Logger()
{
    m_threadPool.Initialize();
}

Logger::~Logger()
{
    m_threadPool.Shutdown();
}

void Logger::Log(LogType p_type, const std::string &p_msg,
	const std::source_location &p_location)
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
        p_msg, status, 
    	std::this_thread::get_id(),
    	p_location.function_name(),
        p_location.file_name(),
        lineNumber
    };

    m_threadPool.PushTask([this, entry = std::move(logEntry)]()  {
        WritingToFile(entry);
    });
}

void Logger::WritingToFile(const LogEntry& logEntry)
{
    auto second = std::chrono::time_point_cast<std::chrono::seconds>(logEntry.timestamp);

    auto local_time = std::chrono::zoned_time{ std::chrono::current_zone(), second };
    std::string timeStr = std::format("{:%Y-%m-%d %H:%M:%S}", local_time);
    std::string threadStr = std::format("{}", logEntry.threadId);

    json j {
        {"Level",     logEntry.status},
        {"Timestamp", timeStr},
        {"Message",   logEntry.msg},
		{"ThreadID",  threadStr},
		{"Function",  logEntry.function},
		{"File",      logEntry.path},
		{"Line",      logEntry.lineNumber}
    };

    {
        std::scoped_lock<std::mutex> lock(GetInstance().m_mutex);
        std::ofstream writingToJsonFile("logger.json", std::ios_base::app);
	    if (writingToJsonFile.is_open())
	    {
		    writingToJsonFile << j.dump(4) << ",\n";
	    }
    }
}

void Logger::ClearLog()
{
	std::scoped_lock<std::mutex> lock(GetInstance().m_mutex);
	std::remove("logger.json");
}