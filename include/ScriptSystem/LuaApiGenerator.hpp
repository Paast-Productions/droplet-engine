#pragma once 

#include <filesystem>
#include <string>
#include <vector>

struct LuaParameterDefinition
{
	std::string name;
	std::string type;
};

struct LuaFunctionDefinition
{
	std::string name;
	std::string returnType;
	std::vector<LuaParameterDefinition> parameters;
};

struct LuaClassDefinition
{
	std::string name;
	std::vector<LuaFunctionDefinition> functions;
};

class LuaApiGenerator
{
public:
	static bool Generate(
		const std::filesystem::path& p_outputPath,
		const std::vector<LuaClassDefinition> p_classes
	);
};