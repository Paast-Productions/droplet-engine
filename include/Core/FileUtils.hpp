/**
* @file FileUtils.hpp
* @brief Utility header for
*/
#pragma once

#include <vector>
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <string_view>


namespace Droplet::Utils
{
	/**
	* @brief Read a file on disc as a char of bytes.
	*/
	inline
		std::vector<char> ReadFileAsBytes(const std::filesystem::path& path)
	{
		const auto filename = path.string();
		const auto onFail = [&filename](std::string_view reason)-> void {
			throw std::runtime_error(std::string(reason) + ":" + filename);
			};

		// open file from the end
		std::ifstream file(path, std::ios::ate | std::ios::binary);

		if (!file.is_open())
			onFail("Failed to open file");

		// we started at the end, so pos == file size
		const auto end = file.tellg();

		if (end < 0)
			onFail("Could not determine file size");


		std::vector<char> buffer(end);

		file.seekg(0, std::ios::beg);

		if (!file)
			onFail("Failed to seek in file");


		file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
		bool result = static_cast<bool>(file);

		if (!buffer.empty() && !result)
		{
			onFail("Failed to read file");
		}

		return buffer;
	}
}
