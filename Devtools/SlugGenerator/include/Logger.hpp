#pragma once
#include <print>
#include <format>
#include <iostream>

namespace Droplet::Devtools::SlugGenerator::Logger
{
	template<typename... Args>
	static void PrintInfo(std::format_string<Args...> fmt, Args &&... args)
	{
		std::print(std::cout, "\033[34mINFO: \033[0m");
		std::println(std::cout, fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	static void PrintWarning(std::format_string<Args...> fmt, Args &&... args)
	{
		std::print(std::cout, "\033[33mWARNING: ");
		std::println(std::cout, fmt, std::forward<Args>(args)...);
		std::print(std::cerr, "\033[0m");
	}

	template<typename... Args>
	static void PrintError(std::format_string<Args...> fmt, Args &&... args)
	{
		std::print(std::cerr, "\033[31mERROR: ");
		std::println(std::cerr, fmt, std::forward<Args>(args)...);
		std::print(std::cerr, "\033[0m");
	}

	template<typename... Args>
	static void FatalError(std::format_string<Args...> fmt, Args &&... args)
	{
		std::print(std::cerr, "\033[31mFATAL ERROR: ");
		std::println(std::cerr, fmt, std::forward<Args>(args)...);
		std::print(std::cerr, "\033[0m");
		throw(std::runtime_error("Fatal error occurred in SlugGenerator."));
	}
	
	static void PrintEmpty()
	{
		std::println("");
	}
}
