#include "Core/IoManager.hpp"
#include "Debug/Logger.hpp"
#include "Core/ThreadPool.hpp"

#include <fstream>
#include <filesystem>
#include <tracy/public/tracy/Tracy.hpp>

namespace Droplet::Core::JsonIO
{
	nlohmann::json Read(const std::string &p_path)
	{
		ZoneScoped;

		std::ifstream fileRead(p_path);
		if (!fileRead.is_open())
		{
			throw std::runtime_error("Failed to open file for reading: " + p_path);
		}

		nlohmann::json j;
		try
		{
			fileRead >> j;
		}
		catch (nlohmann::json::parse_error &e)
		{
			Log(Droplet::Debug::LogType::Error, "Error reading JSON from file: " + p_path + ": " + e.what());
			return nlohmann::json::value_t::discarded;
		}
		return j;
	}

	nlohmann::json ReadRecursive(const fs::path &p_root, const std::string &p_path)
	{
		ZoneScoped;

		// Get the file name we are looking for
		const fs::path targetPath(p_path);
		const std::string targetFilename = targetPath.filename().string();

		// Search recursivley from the root directory
		for (const auto &entry : fs::recursive_directory_iterator(p_root))
		{
			if (entry.is_regular_file() && entry.path().filename() == targetFilename)
			{
				// Found the file, now read it
				return Read(entry.path().string());
			}
		}

		return nlohmann::json();
	}

	void Write(const std::string &p_path, const nlohmann::json &p_data)
	{
		ZoneScoped;

		std::ofstream fileWrite(p_path);
		if (!fileWrite.is_open())
		{
			throw std::runtime_error("Failed to open file for writing: " + p_path);
		}

		try {
			constexpr int indentation = 4; /// Set the desired indentation level for pretty printing
			fileWrite << p_data.dump(indentation) << "\n";
		}
		catch (nlohmann::json::parse_error &e)
		{
			Log(Droplet::Debug::LogType::Error, "Error writing JSON to file: " + p_path + ": " + e.what());
		}
	}

	nlohmann::json ModifyWithAction(const std::string &p_path, const nlohmann::json &p_data, ModifyAction p_modifyAction)
	{
		
		nlohmann::json j {};
		std::ifstream fileRead(p_path);
		if (!fileRead.is_open())
		{
			throw std::runtime_error("Failed to open file for reading: " + p_path);
		}
		try
		{
			fileRead >> j;
		}
		catch (nlohmann::json::parse_error &e)
		{
			Log(Droplet::Debug::LogType::Error, "Error reading JSON from file: " + p_path + ": " + e.what());
			return nlohmann::json::value_t::discarded;
		}
		fileRead.close();
		

		for (const auto &[key, value] : p_data.items())
		{
			switch (p_modifyAction)
			{
			case ModifyAction::Insert:	j[key]= value;	break;
			case ModifyAction::Delete:	j.erase(key);		break;
			}
		}

		std::ofstream fileWrite(p_path);

		if (!fileWrite.is_open())
		{
			throw std::runtime_error("Failed to open file for writing: " + p_path);
		}
		try {
			constexpr int indentation = 4; /// Set the desired indentation level for pretty printing
			fileWrite << j.dump(indentation) << "\n";
		}
		catch (nlohmann::json::parse_error &e)
		{	
			Log(Droplet::Debug::LogType::Error, "Error writing JSON to file: " + p_path + ": " + e.what());
			return nlohmann::json::value_t::discarded;
		}
		fileWrite.close();
		return j;
	}

	void DeleteFile(const std::string &path)
	{
		static_cast<void>(std::remove(path.c_str()));
	}

	void CreateFile(const std::string &path)
	{
		if (std::filesystem::exists(path))
		{
			return; // File already exists, no need to create it
		}

		// Get the parent directory of the file path
		std::filesystem::path parentDir = std::filesystem::path(path).parent_path();

		if (!std::filesystem::is_directory(parentDir) || !std::filesystem::exists(parentDir))
		{
			std::filesystem::create_directory(parentDir);
		}

		std::ofstream file(path);
		if (!file.is_open())
		{
			throw std::runtime_error("Failed to create file: " + path);
		}
		file.close();
	}
}
