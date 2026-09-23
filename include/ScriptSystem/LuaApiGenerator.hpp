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

struct LuaGlobalFunctionDefinition
{
	std::string name;
	std::string returnType;
	std::vector<LuaParameterDefinition> parameters;
};


class LuaApiGenerator
{
public:
	static bool Generate(
		const std::filesystem::path& p_outputPath,
		const std::vector<LuaGlobalFunctionDefinition>& p_globals,
		const std::vector<LuaClassDefinition>& p_classes
	);

private:
	static void GenerateGlobal(
		std::ofstream& p_file,
		const std::vector<LuaGlobalFunctionDefinition> p_global
	);
	static void GenerateClasses(
		std::ofstream& p_file,
		const std::vector<LuaClassDefinition>& p_classes
	);

};