#pragma once
#include <json/json.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace Droplet::Core
{
	/// @class IOManager
	/// @brief The I/O Manager handles reading from and writing to JSON files.
	class IOManager
	{

	public: 
		/// @brief More structs can be added as the codebase develops. 
		/// @brief Example data stored for player.
		struct PlayerSaveData
		{
			std::string name = "Player";
			int level = 1;
			float health = 100.0f;

			// This macro helps the user read from and write to the JSON file.
			NLOHMANN_DEFINE_TYPE_INTRUSIVE(PlayerSaveData, name, level, health)
		};

	public:
		IOManager() = default;
		~IOManager() = default;

		/// @brief Reads data from a JSON file.
		/// @param p_writeFromFile The name of the JSON file to read from.
		/// @param p_data The data to be loaded from the JSON file.
		template <typename T>
		static void Load(std::string &p_writeFromFile, T &p_data)
		{
			//throw
			try
			{
				std::ifstream fileRead(p_writeFromFile);
				if (!fileRead.is_open())
				{
					throw std::runtime_error("Failed to open file for reading: " + p_writeFromFile);
				}

				nlohmann::json j;
				fileRead >> j;
				p_data = j.get<T>();

			}
			catch (nlohmann::json::parse_error& ex)
			{
				throw std::runtime_error("JSON parse error in " + p_writeFromFile + ": " + ex.what());
			}
		}

		/// @brief Writes data to a JSON file.
		/// @param p_readFromFile The name of the JSON file to write to.
		/// @param p_data The data to be loaded to the JSON file.
		template<typename T>
		static bool Save(std::string& p_readFromFile, T &p_data)
		{
			try
			{
				std::ofstream fileWrite(p_readFromFile);

				const nlohmann::json j = p_data;

				if (!fileWrite.is_open())
				{
					throw std::runtime_error("Failed to open file for writing: " + p_readFromFile);
				}
				fileWrite << j.dump(4) << "\n";
				fileWrite.close();
				return true;
			}
			catch (nlohmann::json::parse_error& ex)
			{
				throw std::runtime_error("Failed to serialize data to " + p_readFromFile + ": " + ex.what());
			}
		}
	};
}
