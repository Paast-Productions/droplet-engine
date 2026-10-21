#pragma once
#include <print>
#include <format>
#include <iostream>

namespace Droplet::Devtools::SlugGenerator::Logger
{
	template<typename... Args>
	static void PrintInfo(std::format_string<Args...> fmt, Args &&... args)
	{
		std::print(std::cout, "INFO: ");
		std::println(std::cout, fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	static void PrintWarning(std::format_string<Args...> fmt, Args &&... args)
	{
		std::print(std::cout, "WARNING: ");
		std::println(std::cout, fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	static void PrintError(std::format_string<Args...> fmt, Args &&... args)
	{
		std::print(std::cerr, "ERROR: ");
		std::println(std::cerr, fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	static void FatalError(std::format_string<Args...> fmt, Args &&... args)
	{
		std::print(std::cerr, "FATAL ERROR: ");
		std::println(std::cerr, fmt, std::forward<Args>(args)...);
		throw(std::runtime_error("Fatal error occurred in SlugGenerator."));
	}
}
