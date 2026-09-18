#include "Logger.hpp"

#include <format>
#include <fstream>
#include <thread>

#include <json/json.hpp>

using json = nlohmann::json;

using namespace Droplet::Debug;

Logger::Logger()
{
    m_workerThread = std::thread(&Logger::ProcessQueue, this);
}

Logger::~Logger()
{
    m_running = false;
    m_conditionVariable.notify_all();
    if (m_workerThread.joinable())
        m_workerThread.join();
}

void Logger::Log(LogType p_type, const std::string &p_msg)
{
    std::string s_status{};
    switch (p_type)
    {
    case LogType::Error:   s_status = "Error"; break;
    case LogType::Warning: s_status = "Warning"; break;
    case LogType::Info:    s_status = "Info"; break;
    case LogType::Debug:   s_status = "Debug"; break;
    }

    LogEntry logEntry{ std::chrono::system_clock::now(),
        p_msg, s_status, std::this_thread::get_id() };

    Push(logEntry);
}

void Logger::ProcessQueue()
{
    LogEntry logEntry;
    while (Pop(logEntry) || m_running)
    {
        auto second = std::chrono::time_point_cast<std::chrono::seconds>(logEntry.timestamp);

        auto local_time = std::chrono::zoned_time{ std::chrono::current_zone(), second };
        std::string timeStr = std::format("{:%Y-%m-%d %H:%M:%S}", local_time);
        std::string threadStr = std::format("{}", std::this_thread::get_id());

        json j = nlohmann::json{
            {"Level",     logEntry.status},
            {"Timestamp", timeStr},
            {"Message",   logEntry.msg},
            {"ThreadID",  threadStr}
        };

        std::ofstream writingToJsonFile("logger.json", std::ios_base::app);
        if (writingToJsonFile.is_open())
        {
            writingToJsonFile << j.dump(4) << ",\n";
        }
        writingToJsonFile.close();
    }
}

void Logger::ClearLog()
{
    std::scoped_lock<std::mutex> lock(GetInstance().m_mutex);
    std::ofstream writingToJsonFile("Logger.json", std::ios_base::out | std::ios_base::trunc);
    writingToJsonFile.close();
}

void Logger::Push(LogEntry &p_in)
{
    {
	    std::scoped_lock<std::mutex> lock(GetInstance().m_mutex);
	    m_queue.push(std::move(p_in));
    }
    m_conditionVariable.notify_one();
}

bool Logger::Pop(LogEntry &p_out)
{

    std::unique_lock<std::mutex> lock(GetInstance().m_mutex);
    m_conditionVariable.wait(lock, [this] {return !m_queue.empty() || !m_running;});

    if (m_queue.empty()) return false;
    
    p_out = std::move(m_queue.front());
    m_queue.pop();
    return true;
}
