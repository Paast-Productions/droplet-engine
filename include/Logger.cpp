#include <format>
#include "Logger.hpp"

void logger::Log(LOG_TYPE type, const std::string &msg)
{
    std::scoped_lock<std::mutex> lock(m_g_i_mutex);
    std::string s_status{};
    switch (type)
    {
    case Error:   s_status = "Error"; break;
    case Warning: s_status = "Warning"; break;
    case Info:    s_status = "Info"; break;
    case Debug:   s_status = "Debug"; break;
    }

    auto now = std::chrono::system_clock::now();
    auto second = std::chrono::time_point_cast<std::chrono::seconds>(now);
    auto local_time = std::chrono::zoned_time{ std::chrono::current_zone(), second };

    std::string s_timeStr = std::format("{:%Y-%m-%d %H:%M:%S}", local_time);
    std::string s_threadStr = std::format("{}", std::this_thread::get_id());

    json j = nlohmann::json{
        {"Level",     s_status},
        {"Timestamp", s_timeStr},
        {"Message",   msg},
        {"ThreadID",  s_threadStr}
    };

    std::ofstream writingToJsonFile("logger.json", std::ios_base::app);
    if (writingToJsonFile.is_open())
    {
        writingToJsonFile << j.dump(4) << ",\n";
    }
    writingToJsonFile.close();
}

void logger::ClearLog()
{
	std::scoped_lock<std::mutex> lock(m_g_i_mutex);
    std::ofstream writingToJsonFile("Logger.json", std::ios_base::out | std::ios_base::trunc);
    writingToJsonFile.close();
}