#include "LuaApiGenerator.hpp"

#include <fstream>
#include <print>

bool LuaApiGenerator::Generate(const std::filesystem::path& p_outputPath, 
    const std::vector<LuaGlobalFunctionDefinition>& p_globals, 
    const std::vector<LuaClassDefinition>& p_classes)
{
    std::ofstream file(p_outputPath);

    if (!file.is_open()) 
    {
        std::print("Failed to create Lua API definition file: {}\n", p_outputPath.string());

        return false;
    }


    GenerateGlobal(file, p_globals);
    GenerateClasses(file, p_classes);

    file.close();

    return true;
}

void LuaApiGenerator::GenerateGlobal(std::ofstream& p_file,
    const std::vector<LuaGlobalFunctionDefinition> p_globalFunctions)
{
    for (const LuaGlobalFunctionDefinition& function : p_globalFunctions)
    {
        for (const LuaParameterDefinition& parameter : function.parameters)
        {
            p_file << "---@param "
                << parameter.name
                << " "
                << parameter.type
                << "\n";
        }

        if (function.returnType != "void")
        {
            p_file << "---@return "
                << function.returnType
                << "\n";
        }

        p_file << "function "
            << function.name
            << "(";

        for (std::size_t i = 0; i < function.parameters.size(); ++i)
        {
            p_file << function.parameters[i].name;

            if (i + 1 < function.parameters.size())
            {
                p_file << ", ";
            }
        }

        p_file << ") end\n\n";
    }
}

void LuaApiGenerator::GenerateClasses(std::ofstream& p_file, 
    const std::vector<LuaClassDefinition>& p_classes)
{
    for (const LuaClassDefinition& luaClass : p_classes)
    {
        p_file << "---@class " << luaClass.name << "\n";
        p_file << luaClass.name << " = {}\n\n";

        for (const LuaFunctionDefinition& luaFunction : luaClass.functions)
        {
            for (const LuaParameterDefinition& luaParameter : luaFunction.parameters)
            {
                p_file << "---@param "
                    << luaParameter.name
                    << " "
                    << luaParameter.type
                    << "\n";
            }

            if (luaFunction.returnType != "void")
            {
                p_file << "---@return "
                    << luaFunction.returnType
                    << "\n";
            }

            p_file << "function "
                << luaClass.name
                << luaFunction.name
                << "(";

            for (std::size_t i = 0; i < luaFunction.parameters.size(); ++i)
            {
                p_file << luaFunction.parameters[i].name;

                if (i + 1 < luaFunction.parameters.size())
                {
                    p_file << ", ";
                }
            }

            p_file << ") end\n\n";
        }
    }
}
