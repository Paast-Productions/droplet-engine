#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace Droplet::Editor
{
	/// @brief Class that manages the editor config file.
	/// 
	/// Provides a simple interface for saving and loading editor data.
	/// Data is stored as a key-value pair, where the key is a string and the value is one of many supported types.
	/// 
	/// Data can be sectioned by using a dot notation in the key, for example: "section.subsection.key".
	/// This can help to avoid accidental duplicate keys in different parts of the editor.
	class EditorConfig
	{
	public:

		/// @brief Gets the path to the editor config file.
		/// @return The path to the editor config file.
		static const std::string &GetConfigPath()
		{
			return GetInstance().m_configFilePath;
		}

		/// @brief Sets the path to the editor config file.
		/// @param p_newPath The new path to the editor config file.
		static void SetConfigPath(const std::string &p_newPath)
		{
			GetInstance().m_configFilePath = p_newPath;
		}

		static void						StoreInt(const std::string &p_key,			int p_value);
		static void						StoreLong(const std::string &p_key,			long p_value);
		static void						StoreFloat(const std::string &p_key,		float p_value);
		static void						StoreBool(const std::string &p_key,			bool p_value);
		static void						StoreString(const std::string &p_key,		const std::string &p_value);
		static void						StoreVec2(const std::string &p_key,			const glm::vec2 &p_value);
		static void						StoreVec3(const std::string &p_key,			const glm::vec3 &p_value);
		static void						StoreVec4(const std::string &p_key,			const glm::vec4 &p_value);
		static void						StoreIntArray(const std::string &p_key,		const std::vector<int> &p_value);
		static void						StoreFloatArray(const std::string &p_key,	const std::vector<float> &p_value);
		static void						StoreBoolArray(const std::string &p_key,	const std::vector<bool> &p_value);
		static void						StoreStringArray(const std::string &p_key,	const std::vector<std::string> &p_value);
		static void						StoreVec2Array(const std::string &p_key,	const std::vector<glm::vec2> &p_value);
		static void						StoreVec3Array(const std::string &p_key,	const std::vector<glm::vec3> &p_value);
		static void						StoreVec4Array(const std::string &p_key,	const std::vector<glm::vec4> &p_value);

		static int						LoadInt(const std::string &p_key,			int p_default = 0);
		static long						LoadLong(const std::string &p_key,			long p_default = 0);
		static float					LoadFloat(const std::string &p_key,			float p_default = 0.0f);
		static bool						LoadBool(const std::string &p_key,			bool p_default = false);
		static std::string				LoadString(const std::string &p_key,		const std::string &p_default = "");
		static glm::vec2				LoadVec2(const std::string &p_key,			const glm::vec2 &p_default = glm::vec2(0.0f));
		static glm::vec3				LoadVec3(const std::string &p_key,			const glm::vec3 &p_default = glm::vec3(0.0f));
		static glm::vec4				LoadVec4(const std::string &p_key,			const glm::vec4 &p_default = glm::vec4(0.0f));
		static std::vector<int>			LoadIntArray(const std::string &p_key,		const std::vector<int> &p_default = {});
		static std::vector<float>		LoadFloatArray(const std::string &p_key,	const std::vector<float> &p_default = {});
		static std::vector<bool>		LoadBoolArray(const std::string &p_key,		const std::vector<bool> &p_default = {});
		static std::vector<std::string>	LoadStringArray(const std::string &p_key,	const std::vector<std::string> &p_default = {});
		static std::vector<glm::vec2>	LoadVec2Array(const std::string &p_key,		const std::vector<glm::vec2> &p_default = {});
		static std::vector<glm::vec3>	LoadVec3Array(const std::string &p_key,		const std::vector<glm::vec3> &p_default = {});
		static std::vector<glm::vec4>	LoadVec4Array(const std::string &p_key,		const std::vector<glm::vec4> &p_default = {});

	private:
		/// @brief Gets the singleton instance of the EditorConfig class.
		/// @return The singleton instance of the EditorConfig class.
		static EditorConfig &GetInstance()
		{
			static EditorConfig instance;
			return instance;
		}

		std::string m_configFilePath = "Config/EditorConfig.json";
	};
}