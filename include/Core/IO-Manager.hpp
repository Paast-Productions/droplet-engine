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

	public:
		/// @brief Defines the type of modification action to perform on a JSON file.
		/// insert: Adds new data to the JSON file.
		/// delete: Removes existing data from the JSON file.
		enum class ModifyAction
		{
			Insert,
			Delete,
		};

	public:
		IOManager() = default;
		~IOManager() = default;

		/// @brief Reads data from a JSON file.
		/// @param p_path The name of the JSON file to read from.
		static nlohmann::json Load(const std::string &p_path);
		

		/// @brief Writes data to a JSON file.
		/// @param p_path The name of the JSON file to write to.
		/// @param p_data The data to be loaded to the JSON file.
		static bool Save(const std::string &p_path, const nlohmann::json &p_data);

		/// @brief Modifies data in a JSON file based on the specified action.
		/// @param p_path The name of the JSON file to modify.
		/// @param p_data The data to be used for the modification.
		/// @param p_modifyAction The action to perform (insert or delete).
		static void DataAction(const std::string &p_path, 
			const nlohmann::json &p_data, ModifyAction p_modifyAction);
	};
};

