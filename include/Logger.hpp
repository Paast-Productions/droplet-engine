#pragma once

//namespace Engine
//{
#include <fstream>


#define LOG_ERROR(msg) ::logger::Log(::logger::LOG_TYPE::Error, msg)
#define LOG_WARNING(msg) ::logger::Log(::logger::LOG_TYPE::Warning, msg)
#define LOG_INFO(msg) ::logger::Log(::logger::LOG_TYPE::Info, msg)
#define LOG_DEBUG(msg) ::logger::Log(::logger::LOG_TYPE::Debug, msg)

class logger
{
public:
	inline static std::ofstream writingToFile{};
public:
	enum LOG_TYPE
	{
		Error,
		Warning,
		Info,
		Debug
	};

public:
	logger() = default;
	~logger();
public:
	static void Log(LOG_TYPE type, const std::string& msg);
	static void clearLog();
};

//}
