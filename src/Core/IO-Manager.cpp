#include "Core/IO-Manager.hpp"

namespace Droplet::Core
{
	nlohmann::json IOManager::Load(const std::string &p_path)
	{
		try
		{
			std::ifstream fileRead(p_path);
			if (!fileRead.is_open())
			{
				throw std::runtime_error("Failed to open file for reading: " + p_path);
			}

			nlohmann::json j;
			fileRead >> j;

			return j;
		}
		catch (nlohmann::json::parse_error &ex)
		{
			throw std::runtime_error("JSON parse error in " + p_path + ": " + ex.what());
		}
	}

	bool IOManager::Save(const std::string &p_path, const nlohmann::json &p_data)
	{
		try
		{
			std::ofstream fileWrite(p_path);

			if (!fileWrite.is_open())
			{
				throw std::runtime_error("Failed to open file for writing: " + p_path);
			}

			const nlohmann::json j = p_data;
			fileWrite << j.dump(4) << "\n";
			fileWrite.close();
			return true;
		}
		catch (nlohmann::json::parse_error &ex)
		{
			throw std::runtime_error("Failed to serialize data to " + p_path + ": " + ex.what());
		}
	}

	void IOManager::DataAction(const std::string &p_addToFile, const nlohmann::json &p_data, ModifyAction p_modifyAction)
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

			for (const auto &[key, value] : p_data.items())
			{
				switch (p_modifyAction)
				{
				case ModifyAction::Insert:	j[key] = value; break;
				case ModifyAction::Delete:	j.erase(key);		break;
				}
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
}
