#pragma once
#include <mutex>
#include <chrono>
#include <string>
#include <queue>

namespace Droplet::Debug
{
	/// @brief With the information that the queue holds.
	struct LogEntry
	{
		std::chrono::system_clock::time_point timestamp{};
		std::string msg{};
		std::string status{};
		std::thread::id threadId{};
	};


	/// @class logger
	/// @brief Thread-safe logger class for writing formatted JSON logs.
	/// 
	/// Provides thread-safe logging functionality to capture timestamped
	/// messages and thread IDs to a JSON file.

	class Logger
	{
	public:
		
		/// @brief Defines the log severity level selected for an entry.		 
		enum class LogType
		{
			Error,   /// Critical error events that require attention.
			Warning, /// Warning events indicating potential issues.
			Info,    /// Informational messages about system operation.
			Debug    /// Detailed information for debugging purposes.
		};

		Logger();

		~Logger();

		/// @brief Main logging function, typically invoked via logging macros.
		/// Serializes and appends a log entry to the target JSON file.
		/// @param p_type Severity level of the log entry.
		/// @param p_msg The message text to be logged.
		void Log(LogType p_type,const std::string &p_msg);

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
		/// @param p_in The log entry to be pushed onto the queue.
		void Push(LogEntry &p_in);

		/// @brief Pops a log entry from the queue in a thread-safe manner.
		/// Safely retrieves and removes the next log entry from the queue for
		/// processing by the worker thread.
		/// @param p_out Reference where the popped log entry will be stored.
		/// @return True if an entry was popped successfully, false if the queue is empty.
		bool Pop(LogEntry &p_out);

	private:
		std::mutex m_mutex{};
		std::queue<LogEntry> m_queue{};
		std::condition_variable m_conditionVariable{};
		std::thread m_workerThread{};
		std::atomic<bool> m_running{ true };
	};
}
