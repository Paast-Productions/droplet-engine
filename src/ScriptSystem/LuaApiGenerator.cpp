#include "LuaApiGenerator.hpp"

#include <fstream>
#include <print>

bool LuaApiGenerator::Generate(const std::filesystem::path& p_outputPath, 
    [[maybe_unused]] const std::vector<LuaClassDefinition> p_classes)
{

    std::print(
        "Generating Lua API at: {}\n",
        std::filesystem::absolute(p_outputPath).string()
    );

    std::ofstream file(p_outputPath);

    if (!file.is_open()) {
        std::print("Failed to create Lua API definition file: {}\n", p_outputPath.string());

        return false;
    }
    
    for (const LuaClassDefinition& luaClass : p_classes)
    {
        file << "---@class " << luaClass.name << "\n";
        file << luaClass.name << " = {}\n\n";

        for (const LuaFunctionDefinition& luaFunction : luaClass.functions)
        {
            for (const LuaParameterDefinition& luaParameter : luaFunction.parameters)
            {
                file << "---@param "
                     << luaParameter.name
                     << " "
                     << luaParameter.type
                     << "\n";
            }

            if (luaFunction.returnType != "void")
            {
                file << "---@return "
                     << luaFunction.returnType
                     << "\n";
            }

            file << "function "
                 << luaClass.name
                 << ":"
                 << luaFunction.name
                 << "(";

            for (std::size_t i = 0; i < luaFunction.parameters.size(); ++i)
            {
                file << luaFunction.parameters[i].name;

                if (i + 1 < luaFunction.parameters.size())
                {
                    file << ", ";
                }
            }

            file << ") end\n\n";
        }
    }


    file.close();

    std::print("Generated Lua API definition: {}\n", p_outputPath.string());

    return true;
}
