#pragma once
#include <mutex>
#include <chrono>
#include <string>
#include <queue>

#include <json/json.hpp>

using json = nlohmann::json;

/**
 * @class logger
 * @brief Thread-safe logger class for writing formatted JSON logs.
 *
 * Provides thread-safe logging functionality to capture timestamped
 * messages and thread IDs to a JSON file.
 */


 /**
 * @brief Convenience macro to log an error message.
 * @param msg The message string to log.
 */
#define LOG_ERROR(msg)   ::Logger::GetInstance().Log(::Logger::LOG_TYPE::Error, msg)
#define LOG_WARNING(msg) ::Logger::GetInstance().Log(::Logger::LOG_TYPE::Warning, msg)
#define LOG_INFO(msg)    ::Logger::GetInstance().Log(::Logger::LOG_TYPE::Info, msg)
#define LOG_DEBUG(msg)   ::Logger::GetInstance().Log(::Logger::LOG_TYPE::Debug, msg)


struct LogEntry
{
	std::chrono::system_clock::time_point timestamp;
	std::string msg;
	std::string status;
	std::thread::id threadId;
};

class Logger
{
public:
	/**
	 * @enum LOG_TYPE
	 * @brief Defines the log severity level selected for an entry.
	 */
	enum LOG_TYPE
	{
		Error,   ///< Critical error events that require attention.
		Warning, ///< Warning events indicating potential issues.
		Info,    ///< Informational messages about system operation.
		Debug    ///< Detailed information for debugging purposes.
	};

	Logger() = default;

	~Logger() = default;

	/**
	 * @brief Main logging function, typically invoked via logging macros.
	 * Serializes and appends a log entry to the target JSON file.
	 *
	 * @param type log entry (e.g., Error, Warning, Info, Debug).
	 * @param msg The message text to be logged.
	 */
	void Log(LOG_TYPE type, const std::string &msg);
	/**
	 * @brief Clears all content from the log file and close the file.
	 */

	void PrintToLog(LogEntry s_logEntry);
	static void ClearLog();

	// How to get the instance in a singleton pattern
	static Logger &GetInstance()
	{
		static Logger logger;
		return logger;
	}

private:
	void Push(LogEntry &in);
	bool Pop(LogEntry &out);

private:
	std::mutex m_mutex;
	std::queue<LogEntry> m_queue;
};