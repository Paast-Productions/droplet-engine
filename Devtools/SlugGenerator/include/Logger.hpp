#pragma once
#include <print>
#include <format>
#include <iostream>

namespace Droplet::Devtools::SlugGenerator::Logger
{
	/// @brief Prints an informational message to standard output.
	///
	/// Formats the provided message and prefixes it with a blue INFO label.
	///
	/// @param fmt Compile-time checked format string.
	/// @param args Arguments used to format the message.
	template<typename... Args>
	void PrintInfo(std::format_string<Args...> fmt, Args &&... args)
	{
		std::print(std::cout, "\033[34mINFO: \033[0m");
		std::println(std::cout, fmt, std::forward<Args>(args)...);
	}

	/// @brief Prints a warning message to standard output.
	///
	/// Formats the provided message and prefixes it with a yellow WARNING label.
	///
	/// @param fmt Compile-time checked format string.
	/// @param args Arguments used to format the message.
	template<typename... Args>
	void PrintWarning(std::format_string<Args...> fmt, Args &&... args)
	{
		std::print(std::cout, "\033[33mWARNING: \033[0m");
		std::println(std::cout, fmt, std::forward<Args>(args)...);
	}

	/// @brief Prints an error message to standard error.
	///
	/// Formats the provided message and prefixes it with a red ERROR label.
	///
	/// @param fmt Compile-time checked format string.
	/// @param args Arguments used to format the message.
	template<typename... Args>
	void PrintError(std::format_string<Args...> fmt, Args &&... args)
	{
		std::print(std::cerr, "\033[31mERROR: ");
		std::println(std::cerr, fmt, std::forward<Args>(args)...);
		std::print(std::cerr, "\033[0m");
	}

	/// @brief Reports a fatal error and throws an exception.
	///
	/// Formats and prints the error message to standard error before
	/// throwing std::runtime_error containing the same message.
	///
	/// @param fmt Compile-time checked format string.
	/// @param args Arguments used to format the error message.
	///
	/// @throws std::runtime_error Containing the formatted error message.
	template<typename... Args>
	[[noreturn]] void FatalError(std::format_string<Args...> fmt, Args &&... args)
	{
		const std::string message = std::format(fmt, std::forward<Args>(args)... );
		std::println(std::cerr, "\033[31mFATAL ERROR: \033[0m{}", message);
		throw std::runtime_error(message);
	}
	
	/// @brief Prints an empty line to standard output.
	inline void PrintEmpty()
	{
		std::println("");
	}
}
