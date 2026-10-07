#pragma once
#include <json/json.hpp>
#include <stdexcept>

namespace Droplet::Core::JsonIO
{
		/// @brief Defines the type of modification action to perform on a JSON file.
		enum class ModifyAction
		{
			Insert, /// insert: That represents the action of adding new data to the JSON file.
			Delete, /// delete: That represents the action of removing existing data from the JSON file.
		};

		/// @brief Reads data from a JSON file.
		/// @param p_path The name of the JSON file to read from.
		nlohmann::json Read(const std::string &p_path);

		/// @brief Writes data to a JSON file.
		/// @param p_path The name of the JSON file to write to.
		/// @param p_data The data to be loaded to the JSON file.
		void Write(const std::string &p_path, const nlohmann::json &p_data);

		/// @brief Modifies data in a JSON file based on the specified action.
		/// @param p_path The name of the JSON file to modify.
		/// @param p_data The data to be used for the modification.
		/// @param p_modifyAction The action to perform (insert or delete).
		nlohmann::json ModifyWithAction(const std::string &p_path,
			const nlohmann::json &p_data, ModifyAction p_modifyAction);

		/// @brief Deletes all files in the specified directory.
		void DeleteFile(const std::string &path);
};

