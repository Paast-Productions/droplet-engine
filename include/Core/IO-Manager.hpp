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
	private:
		//DO TO test use std::pair and some kind of bucket or list
		//To make it that you can iclude mulitble object. 

	public:
		enum class ModifyAction
		{
			Insert,
			Delete,
			Update,
		};


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
		static T Load(std::string &p_readFromFile)
		{
			//throw
			try
			{
				std::ifstream fileRead(p_readFromFile);
				if (!fileRead.is_open())
				{
					throw std::runtime_error("Failed to open file for reading: " + p_readFromFile);
				}

				nlohmann::json j;
				fileRead >> j;
				
				return j.get<T>();
			}
			catch (nlohmann::json::parse_error& ex)
			{
				throw std::runtime_error("JSON parse error in " + p_readFromFile + ": " + ex.what());
			}
		}

		/// @brief Writes data to a JSON file.
		/// @param p_readFromFile The name of the JSON file to write to.
		/// @param p_data The data to be loaded to the JSON file.
		template<typename T>
		static bool Save(std::string& p_writeFromFile, T &p_data)
		{
			try
			{
				std::ofstream fileWrite(p_writeFromFile);

				if (!fileWrite.is_open())
				{
					throw std::runtime_error("Failed to open file for writing: " + p_writeFromFile);
				}

				const nlohmann::json j = p_data;
				fileWrite << j.dump(4) << "\n";
				fileWrite.close();
				return true;
			}
			catch (nlohmann::json::parse_error& ex)
			{
				throw std::runtime_error("Failed to serialize data to " + p_writeFromFile + ": " + ex.what());
			}
		}

		template<typename T>
		static void DataAction(std::string &p_addToFile,T &p_key,  T &p_data,
			ModifyAction p_modify_action)
		{
			try
			{
				nlohmann::json j;

				{
					std::ifstream fileRead(p_addToFile);
					if (!fileRead.is_open())
					{
						throw std::runtime_error("Failed to open file for reading: " + p_addToFile);
					}

					fileRead >> j;
				}

				switch (p_modify_action)
				{
				case ModifyAction::Insert:	j[p_key] = p_data;	break;
				case ModifyAction::Delete:	j.erase(p_key);	break;
				//case ModifyAction::Update:	j.update();
				}


				{
					std::ofstream fileWrite(p_addToFile);

					if (!fileWrite.is_open())
					{
						throw std::runtime_error("Failed to open file for writing: " + p_addToFile);
					}

					fileWrite << j.dump(4) << "\n";
				}


			}
			catch (nlohmann::json::parse_error &ex)
			{
				throw std::runtime_error("Failed to serialize data to " + p_addToFile + ": " + ex.what());
			}
		}

		
	};
};

