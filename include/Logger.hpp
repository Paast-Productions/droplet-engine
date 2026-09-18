#pragma once
#include <mutex>
#include <chrono>
#include <string>
#include <queue>

#include <json/json.hpp>

using json = nlohmann::json;


/// @class logger
/// @brief Thread-safe logger class for writing formatted JSON logs.
///
/// Provides thread-safe logging functionality to capture timestamped
/// messages and thread IDs to a JSON file.



 
 /// @brief Convenience macro to log an error message.
 /// @param msg The message string to log.
#define LOG_ERROR(msg)   ::Droplet::Logger::GetInstance().Log(::Droplet::Logger::LOG_TYPE::Error, msg)
#define LOG_WARNING(msg) ::Droplet::Logger::GetInstance().Log(::Droplet::Logger::LOG_TYPE::Warning, msg)
#define LOG_INFO(msg)    ::Droplet::Logger::GetInstance().Log(::Droplet::Logger::LOG_TYPE::Info, msg)
#define LOG_DEBUG(msg)   ::Droplet::Logger::GetInstance().Log(::Droplet::Logger::LOG_TYPE::Debug, msg)


struct LogEntry
{
	std::chrono::system_clock::time_point timestamp;
	std::string msg;
	std::string status;
	std::thread::id threadId;
};

namespace Droplet
{
	class Logger
	{
	public:
		
		/// @enum LOG_TYPE
		/// @brief Defines the log severity level selected for an entry.
		 
		enum LOG_TYPE
		{
			Error,   ///< Critical error events that require attention.
			Warning, ///< Warning events indicating potential issues.
			Info,    ///< Informational messages about system operation.
			Debug    ///< Detailed information for debugging purposes.
		};

		Logger();

		~Logger();

		/// @brief Main logging function, typically invoked via logging macros.
		/// Serializes and appends a log entry to the target JSON file.
		/// @param type Severity level of the log entry.
		/// @param msg The message text to be logged.
		void Log(LOG_TYPE type, const std::string &msg);

		/// @brief Processes queued log entries on the background thread.
		void ProcessQueue();

		/// @brief Clears all content from the log file and closes it.
		static void ClearLog();

		/// @brief Retrieves the static instance of the Logger (Meyers Singleton).
		/// @return Reference to the Logger instance.
		static Logger &GetInstance()
		{
			static Logger logger;
			return logger;
		}

	private:
		/// @brief Pushes a log entry onto the queue and signals the worker thread.
		///
		/// Thread-safely adds a new log entry to the queue and notifies the
		/// background worker thread waiting to process incoming data.
		/// @param in The log entry to be pushed onto the queue.
		void Push(LogEntry &in);

		/// @brief Pops a log entry from the queue in a thread-safe manner.
		/// Safely retrieves and removes the next log entry from the queue for
		/// processing by the worker thread.
		/// @param out Reference where the popped log entry will be stored.
		/// @return True if an entry was popped successfully, false if the queue is empty.
		bool Pop(LogEntry &out);

	private:
		std::mutex m_mutex;
		std::queue<LogEntry> m_queue;
		std::condition_variable m_cv;
		std::thread m_workerThread;
		std::atomic<bool> m_running{ true };
	};
}
