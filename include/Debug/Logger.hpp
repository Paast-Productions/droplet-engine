#pragma once
#include <mutex>
#include <chrono>
#include <string>
#include <thread>
#include <source_location>

#include "core/ThreadPool.hpp"

namespace Droplet::Debug
{
	/// @class Logger
	/// @brief Thread-safe logger class for writing formatted JSON logs.
	///
	/// Provides thread-safe logging functionality to capture timestamped
	/// messages and thread IDs to a JSON file.

	class Logger
	{
	private:	/// @brief With the information that the queue holds.
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
	private:
		std::mutex m_mutex{};
		Droplet::ThreadPool &m_threadPool{ Droplet::ThreadPool::GetInstance() };

	public:
		/// @brief Defines the log severity level selected for an entry.		 
		enum class LogType
		{
			Error,   /// Critical error events that require attention.
			Warning, /// Warning events indicating potential issues.
			Info,    /// Informational messages about system operation.
			Debug    /// Detailed information for debugging purposes.
		};
	public:
		Logger();
		~Logger();

		/// @brief Main logging function, typically invoked via logging macros.
		/// Serializes and appends a log entry to the target JSON file.
		/// @param p_type Severity level of the log entry.
		/// @param p_msg The message text to be logged.
		void Log(LogType p_type,const std::string &p_msg, 
			const std::source_location &p_location = std::source_location::current());

		/// @brief Processes queued log entries on the background thread.
		static void WritingToFile(const LogEntry& logEntry);

		/// @brief Clears all content from the log file and closes it.
		static void ClearLog();

	private:
		/// @brief Retrieves the static instance of the Logger (Meyers Singleton).
		/// @return Reference to the Logger instance.
		static Logger &GetInstance()
		{
			static Logger s_logger;
			return s_logger;
		}
	};
}
