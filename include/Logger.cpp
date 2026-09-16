#include <chrono>
#include <iostream>
#include "Logger.hpp"
#include "format"
#include "thread"

//namespace Engine
//{

logger::~logger()
{
	writingToFile.clear();
	writingToFile.close();
}

static void WriteLogEntry(const auto &local_time,
		std::ofstream &writingToFile,
		const std::string &msg,
		std::string_view status)
{
	std::string totalmsg = std::format(
		"{{\n"
		"  \"level\": \"{}\",\n"
		"  \"timestamp\": \"{:%Y-%m-%d %H:%M:%S}\",\n"
		"  \"message\": \"{}\"\n"
		"}},\n",
		status,
		local_time,
		msg
	);
	writingToFile << totalmsg;
}

void logger::Log(LOG_TYPE type, const std::string &msg)
{
	auto now = std::chrono::system_clock::now();

	if (!writingToFile.is_open())
		writingToFile.open("Logger.txt", std::ios_base::app);

	auto second = std::chrono::time_point_cast<std::chrono::seconds>(now);
	auto local_time = std::chrono::zoned_time{ std::chrono::current_zone(), second };

	switch (type)
	{
	case Error:
		WriteLogEntry(local_time, writingToFile, msg, "Error");
		break;
	case Warning:
		WriteLogEntry(local_time, writingToFile, msg, "Warning");
		break;
	case Info:
		WriteLogEntry(local_time, writingToFile, msg, "Info");
		break;
	case Debug:
		WriteLogEntry(local_time, writingToFile, msg, "Debug ");
		break;
	}
}

void logger::clearLog()
{
	if (writingToFile.is_open())
	{
		writingToFile.close();
	}

	writingToFile.open("Logger.txt", std::ios_base::out | std::ios_base::trunc);
	writingToFile.close();

	writingToFile.open("Logger.txt", std::ios_base::app);
}