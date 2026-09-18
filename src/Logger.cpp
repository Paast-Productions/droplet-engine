#include "Logger.hpp"

#include <format>
#include <fstream>
#include <thread>

namespace Droplet
{
    Logger::Logger()
    {
        m_workerThread = std::thread(&Logger::ProcessQueue, this);
    }

    Logger::~Logger()
    {
        m_running = false;
        m_cv.notify_all();
        if (m_workerThread.joinable())
            m_workerThread.join();
    }

    void Logger::Log(LOG_TYPE type, const std::string &msg)
    {
        std::string s_status{};
        switch (type)
        {
        case Error:   s_status = "Error"; break;
        case Warning: s_status = "Warning"; break;
        case Info:    s_status = "Info"; break;
        case Debug:   s_status = "Debug"; break;
        }

        LogEntry s_logEntry{ std::chrono::system_clock::now(),
            msg, s_status, std::this_thread::get_id() };

        Push(s_logEntry);
    }

    void Logger::ProcessQueue()
    {
        LogEntry logEntry;
        while (Pop(logEntry) || m_running)
        {
            auto second = std::chrono::time_point_cast<std::chrono::seconds>(logEntry.timestamp);

            auto local_time = std::chrono::zoned_time{ std::chrono::current_zone(), second };
            std::string s_timeStr = std::format("{:%Y-%m-%d %H:%M:%S}", local_time);
            std::string s_threadStr = std::format("{}", std::this_thread::get_id());

            json j = nlohmann::json{
                {"Level",     logEntry.status},
                {"Timestamp", s_timeStr},
                {"Message",   logEntry.msg},
                {"ThreadID",  s_threadStr}
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

    void Logger::Push(LogEntry &in)
    {
	    {
		    std::scoped_lock<std::mutex> lock(GetInstance().m_mutex);
	    	m_queue.push(std::move(in));
	    }
        m_cv.notify_one();
    }

    bool Logger::Pop(LogEntry &out)
    {

        std::unique_lock<std::mutex> lock(GetInstance().m_mutex);
        m_cv.wait(lock, [this] {return !m_queue.empty() || !m_running;});

        if (m_queue.empty()) return false;
        
        out = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }
}