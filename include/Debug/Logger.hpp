#pragma once
#include <chrono>
#include <string>
#include <thread>
#include <source_location>

namespace Droplet::Debug
{
	/// @brief With the information that the queue holds.
	struct LogEntry
	{
		std::chrono::system_clock::time_point timestamp{};
		std::string msg{};
		std::string status{};
		std::thread::id threadId{};
		std::string function{};
		std::string path{};
		std::string lineNumber{};
	};

	/// @brief Defines the log severity level selected for an entry.		 
	enum class LogType
	{
		Error,   /// Critical error events that require attention.
		Warning, /// Warning events indicating potential issues.
		Info,    /// Informational messages about system operation.
		Debug    /// Detailed information for debugging purposes.
	};

	/// @brief Main logging function, typically invoked via logging macros.
	/// Serializes and appends a log entry to the target JSON file.
	/// @param p_type Severity level of the log entry.
	/// @param p_msg The message text to be logged.
	void Log(LogType p_type, const std::string& p_msg,
		const std::source_location& p_location = std::source_location::current());
}