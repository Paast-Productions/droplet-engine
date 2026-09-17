#pragma once
#include <fstream>
#include <mutex>
#include <chrono>
#include <thread>
#include <string>
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
#define LOG_ERROR(msg)   ::logger::Log(::logger::LOG_TYPE::Error, msg)
#define LOG_WARNING(msg) ::logger::Log(::logger::LOG_TYPE::Warning, msg)
#define LOG_INFO(msg)    ::logger::Log(::logger::LOG_TYPE::Info, msg)
#define LOG_DEBUG(msg)   ::logger::Log(::logger::LOG_TYPE::Debug, msg)


class logger
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

	logger() = default;

	~logger() = default;

	/**
	 * @brief Main logging function, typically invoked via logging macros.
	 * Serializes and appends a log entry to the target JSON file.
	 *
	 * @param type log entry (e.g., Error, Warning, Info, Debug).
	 * @param msg The message text to be logged.
	 */
	static void Log(LOG_TYPE type, const std::string &msg);
	/**
	 * @brief Clears all content from the log file and close the file.
	 */
	static void ClearLog();
private:
	inline static std::mutex m_g_i_mutex;
};