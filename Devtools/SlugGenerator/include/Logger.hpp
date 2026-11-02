#pragma once
#include <print>
#include <format>
#include <iostream>

namespace Droplet::Devtools::SlugGenerator::Logger
{
	template<typename... Args>
	void PrintInfo(std::format_string<Args...> fmt, Args &&... args)
	{
		std::print(std::cout, "\033[34mINFO: \033[0m");
		std::println(std::cout, fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	void PrintWarning(std::format_string<Args...> fmt, Args &&... args)
	{
		std::print(std::cout, "\033[33mWARNING: \033[0m");
		std::println(std::cout, fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	void PrintError(std::format_string<Args...> fmt, Args &&... args)
	{
		std::print(std::cerr, "\033[31mERROR: ");
		std::println(std::cerr, fmt, std::forward<Args>(args)...);
		std::print(std::cerr, "\033[0m");
	}

	template<typename... Args>
	[[noreturn]] void FatalError(std::format_string<Args...> fmt, Args &&... args)
	{
		const std::string message = std::format(fmt, std::forward<Args>(args)... );
		std::println(std::cerr, "\033[31mFATAL ERROR: \033[0m{}", message);
		throw std::runtime_error(message);
	}
	
	inline void PrintEmpty()
	{
		std::println("");
	}
}
