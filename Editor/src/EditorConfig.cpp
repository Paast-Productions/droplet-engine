#include "EditorConfig.hpp"
#include <Core/IoManager.hpp>
#include <json/json.hpp>
#include <tracy/public/tracy/Tracy.hpp>

using namespace Droplet::Editor;
using namespace nlohmann;


static void SaveConfigFile(const json &p_json)
{
	ZoneScoped;

	const std::string &path = EditorConfig::GetConfigPath();
	Droplet::Core::JsonIO::Write(path, p_json);
}

static json OpenConfigFile()
{
	ZoneScoped;

	const std::string &path = EditorConfig::GetConfigPath();

	try
	{
		return Droplet::Core::JsonIO::Read(path);
	}
	catch ([[maybe_unused]] const std::exception &e)
	{
		json j = json::object(); 
		SaveConfigFile(j);
		return j;
	}
}

static std::vector<std::string> SplitKey(const std::string &p_key)
{
	std::vector<std::string> result;
	std::string currentKey;

	for (char c : p_key)
	{
		if (c == '.')
		{
			result.push_back(currentKey);
			currentKey.clear();
		}
		else
		{
			currentKey += c;
		}
	}

	result.push_back(currentKey);
	return result;
}

static bool TraverseToKey(json &p_json, json *&p_out, const std::vector<std::string> &p_keys, bool p_createIfMissing = false)
{
	json *current = &p_json;
	for (size_t i = 0; i < p_keys.size(); ++i)
	{
		if (!current->contains(p_keys[i]) || !(*current)[p_keys[i]].is_object())
		{
			[[unlikely]]
			if (!p_createIfMissing)
			{
				return false;
			}

			(*current)[p_keys[i]] = json::object();
		}

		current = &(*current)[p_keys[i]];
	}

	p_out = current;
	return true;
}


void EditorConfig::StoreInt(const std::string &p_key, int p_value)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	TraverseToKey(config, parent, splitKeys, true);

	(*parent)[lastKey] = p_value;
	SaveConfigFile(config);
}

void EditorConfig::StoreLong(const std::string &p_key, long p_value)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	TraverseToKey(config, parent, splitKeys, true);

	(*parent)[lastKey] = p_value;
	SaveConfigFile(config);
}

void EditorConfig::StoreFloat(const std::string &p_key, float p_value)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	TraverseToKey(config, parent, splitKeys, true);

	(*parent)[lastKey] = p_value;
	SaveConfigFile(config);
}

void EditorConfig::StoreBool(const std::string &p_key, bool p_value)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	TraverseToKey(config, parent, splitKeys, true);

	(*parent)[lastKey] = p_value;
	SaveConfigFile(config);
}

void EditorConfig::StoreString(const std::string &p_key, const std::string &p_value)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	TraverseToKey(config, parent, splitKeys, true);

	(*parent)[lastKey] = p_value;
	SaveConfigFile(config);
}

void EditorConfig::StoreVec2(const std::string &p_key, const glm::vec2 &p_value)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	TraverseToKey(config, parent, splitKeys, true);

	(*parent)[lastKey] = { p_value.x, p_value.y };
	SaveConfigFile(config);
}

void EditorConfig::StoreVec3(const std::string &p_key, const glm::vec3 &p_value)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	TraverseToKey(config, parent, splitKeys, true);

	(*parent)[lastKey] = { p_value.x, p_value.y, p_value.z };
	SaveConfigFile(config);
}

void EditorConfig::StoreVec4(const std::string &p_key, const glm::vec4 &p_value)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	TraverseToKey(config, parent, splitKeys, true);

	(*parent)[lastKey] = { p_value.x, p_value.y, p_value.z, p_value.w };
	SaveConfigFile(config);
}

void EditorConfig::StoreIntArray(const std::string &p_key, const std::vector<int> &p_value)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	TraverseToKey(config, parent, splitKeys, true);

	(*parent)[lastKey] = p_value;
	SaveConfigFile(config);
}

void EditorConfig::StoreFloatArray(const std::string &p_key, const std::vector<float> &p_value)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	TraverseToKey(config, parent, splitKeys, true);

	(*parent)[lastKey] = p_value;
	SaveConfigFile(config);
}

void EditorConfig::StoreBoolArray(const std::string &p_key, const std::vector<bool> &p_value)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	TraverseToKey(config, parent, splitKeys, true);

	(*parent)[lastKey] = p_value;
	SaveConfigFile(config);
}

void EditorConfig::StoreStringArray(const std::string &p_key, const std::vector<std::string> &p_value)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	TraverseToKey(config, parent, splitKeys, true);

	(*parent)[lastKey] = p_value;
	SaveConfigFile(config);
}

void EditorConfig::StoreVec2Array(const std::string &p_key, const std::vector<glm::vec2> &p_value)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json array = json::array();
	for (const auto &val : p_value)
	{
		array.push_back({ val.x, val.y });
	}

	json *parent = nullptr;
	TraverseToKey(config, parent, splitKeys, true);

	(*parent)[lastKey] = array;
	SaveConfigFile(config);
}

void EditorConfig::StoreVec3Array(const std::string &p_key, const std::vector<glm::vec3> &p_value)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json array = json::array();
	for (const auto &val : p_value)
	{
		array.push_back({ val.x, val.y, val.z });
	}

	json *parent = nullptr;
	TraverseToKey(config, parent, splitKeys, true);

	(*parent)[lastKey] = array;
	SaveConfigFile(config);
}

void EditorConfig::StoreVec4Array(const std::string &p_key, const std::vector<glm::vec4> &p_value)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json array = json::array();
	for (const auto &val : p_value)
	{
		array.push_back({ val.x, val.y, val.z, val.w });
	}

	json *parent = nullptr;
	TraverseToKey(config, parent, splitKeys, true);

	(*parent)[lastKey] = array;
	SaveConfigFile(config);
}


int EditorConfig::LoadInt(const std::string &p_key, int p_default)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	if (!TraverseToKey(config, parent, splitKeys))
	{
		return p_default;
	}

	return (*parent).value(lastKey, p_default);
}

long EditorConfig::LoadLong(const std::string &p_key, long p_default)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	if (!TraverseToKey(config, parent, splitKeys))
	{
		return p_default;
	}

	return (*parent).value(lastKey, p_default);
}

float EditorConfig::LoadFloat(const std::string &p_key, float p_default)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	if (!TraverseToKey(config, parent, splitKeys))
	{
		return p_default;
	}

	return (*parent).value(lastKey, p_default);
}

bool EditorConfig::LoadBool(const std::string &p_key, bool p_default)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	if (!TraverseToKey(config, parent, splitKeys))
	{
		return p_default;
	}

	return (*parent).value(lastKey, p_default);
}

std::string EditorConfig::LoadString(const std::string &p_key, const std::string &p_default)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	if (!TraverseToKey(config, parent, splitKeys))
	{
		return p_default;
	}

	return (*parent).value(lastKey, p_default);
}

glm::vec2 EditorConfig::LoadVec2(const std::string &p_key, const glm::vec2 &p_default)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	if (!TraverseToKey(config, parent, splitKeys))
	{
		return p_default;
	}

	json &value = (*parent)[lastKey];

	[[unlikely]]
	if (!value.is_array() || value.size() < 2)
	{
		return p_default;
	}

	return {
		value[0],
		value[1]
	};
}

glm::vec3 EditorConfig::LoadVec3(const std::string &p_key, const glm::vec3 &p_default)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	if (!TraverseToKey(config, parent, splitKeys))
	{
		return p_default;
	}

	json &value = (*parent)[lastKey];

	[[unlikely]]
	if (!value.is_array() || value.size() < 3)
	{
		return p_default;
	}

	return {
		value[0],
		value[1],
		value[2]
	};
}

glm::vec4 EditorConfig::LoadVec4(const std::string &p_key, const glm::vec4 &p_default)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	if (!TraverseToKey(config, parent, splitKeys))
	{
		return p_default;
	}

	json &value = (*parent)[lastKey];

	[[unlikely]]
	if (!value.is_array() || value.size() < 4)
	{
		return p_default;
	}

	return {
		value[0],
		value[1],
		value[2],
		value[3]
	};
}

std::vector<int> EditorConfig::LoadIntArray(const std::string &p_key, const std::vector<int> &p_default)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	if (!TraverseToKey(config, parent, splitKeys))
	{
		return p_default;
	}

	json &value = (*parent)[lastKey];

	[[unlikely]]
	if (!value.is_array())
	{
		return p_default;
	}

	return value.get<std::vector<int>>();
}

std::vector<float> EditorConfig::LoadFloatArray(const std::string &p_key, const std::vector<float> &p_default)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	if (!TraverseToKey(config, parent, splitKeys))
	{
		return p_default;
	}

	json &value = (*parent)[lastKey];

	[[unlikely]]
	if (!value.is_array())
	{
		return p_default;
	}

	return value.get<std::vector<float>>();
}

std::vector<bool> EditorConfig::LoadBoolArray(const std::string &p_key, const std::vector<bool> &p_default)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	if (!TraverseToKey(config, parent, splitKeys))
	{
		return p_default;
	}
	
	json &value = (*parent)[lastKey];

	[[unlikely]]
	if (!value.is_array())
	{
		return p_default;
	}

	return value.get<std::vector<bool>>();
}

std::vector<std::string> EditorConfig::LoadStringArray(const std::string &p_key, const std::vector<std::string> &p_default)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	if (!TraverseToKey(config, parent, splitKeys))
	{
		return p_default;
	}

	json &value = (*parent)[lastKey];

	[[unlikely]]
	if (!value.is_array())
	{
		return p_default;
	}

	return value.get<std::vector<std::string>>();
}

std::vector<glm::vec2> EditorConfig::LoadVec2Array(const std::string &p_key, const std::vector<glm::vec2> &p_default)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	if (!TraverseToKey(config, parent, splitKeys))
	{
		return p_default;
	}

	json &value = (*parent)[lastKey];

	[[unlikely]]
	if (!value.is_array())
	{
		return p_default;
	}

	std::vector<glm::vec2> result;
	result.reserve(value.size());

	for (const auto &item : value)
	{
		[[unlikely]]
		if (!item.is_array() || item.size() < 2)
		{
			continue;
		}
		result.emplace_back(item[0], item[1]);
	}

	return result;
}

std::vector<glm::vec3> EditorConfig::LoadVec3Array(const std::string &p_key, const std::vector<glm::vec3> &p_default)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	if (!TraverseToKey(config, parent, splitKeys))
	{
		return p_default;
	}

	json &value = (*parent)[lastKey];

	[[unlikely]]
	if (!value.is_array())
	{
		return p_default;
	}

	std::vector<glm::vec3> result;
	result.reserve(value.size());

	for (const auto &item : value)
	{
		[[unlikely]]
		if (!item.is_array() || item.size() < 3)
		{
			continue;
		}
		result.emplace_back(item[0], item[1], item[2]);
	}

	return result;
}

std::vector<glm::vec4> EditorConfig::LoadVec4Array(const std::string &p_key, const std::vector<glm::vec4> &p_default)
{
	[[unlikely]]
	if (p_key.empty())
	{
		throw std::invalid_argument("Key cannot be empty.");
	}

	std::vector<std::string> splitKeys = SplitKey(p_key);
	const std::string lastKey = splitKeys.back();
	splitKeys.pop_back();

	json config = OpenConfigFile();

	json *parent = nullptr;
	if (!TraverseToKey(config, parent, splitKeys))
	{
		return p_default;
	}

	json &value = (*parent)[lastKey];

	[[unlikely]]
	if (!value.is_array())
	{
		return p_default;
	}

	std::vector<glm::vec4> result;
	result.reserve(value.size());

	for (const auto &item : value)
	{
		[[unlikely]]
		if (!item.is_array() || item.size() < 4)
		{
			continue;
		}
		result.emplace_back(item[0], item[1], item[2], item[3]);
	}

	return result;
}
