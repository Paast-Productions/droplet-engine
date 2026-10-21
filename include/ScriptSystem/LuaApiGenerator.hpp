#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <fstream>

namespace Droplet::Script
{
	/// @brief Describes a parameter of a Lua function.
	/// Stores the name and type information used by the Lua API generator when
	/// generating function declarations.
	struct LuaParameterDefinition
	{
		/// @brief Name of the parameter.
		std::string name;

		/// @brief Lua type of the parameter.
		std::string type;

		bool optional = false;

		LuaParameterDefinition(const std::string &p_name, const std::string &p_type)
			: name(p_name), type(p_type) { }

		LuaParameterDefinition(const std::string &p_name, const std::string &p_type, bool p_optional)
			: name(p_name), type(p_type), optional(p_optional) { }
	};

	/// @brief Describes a member function of a Lua-exposed class.
	/// Contains the function name, return type, and parameters required to
	/// generate the corresponding Lua API declaration.
	struct LuaFunctionDefinition
	{
		/// @brief Name of the function.
		std::string name;

		/// @brief Return type of the function.
		std::string returnType;

		/// @brief Parameters accepted by the function.
		std::vector<LuaParameterDefinition> parameters;

		bool isStatic = false;

		LuaFunctionDefinition(const std::string &p_name, const std::string &p_returnType, const std::vector<LuaParameterDefinition> &p_parameters)
			: name(p_name), returnType(p_returnType), parameters(p_parameters) { }

		LuaFunctionDefinition(const std::string &p_name, const std::string &p_returnType, bool p_isStatic, const std::vector<LuaParameterDefinition> &p_parameters)
			: name(p_name), returnType(p_returnType), parameters(p_parameters), isStatic(p_isStatic) { }
	};

	/// @brief Describes an enum flag exposed through the Lua API.
	/// Contains the name of the flag and its corresponding value 
	/// required to generate the corresponding Lua API declaration.
	struct LuaFlagDefinition
	{
		/// @brief Name of the flag.
		std::string name;

		/// @brief Value of the flag.
		std::size_t value;
	};

	/// @brief Describes a class exposed through the Lua API.
	/// Contains the name of the class and the functions that are available
	/// through the generated Lua API definition.
	struct LuaClassDefinition
	{
		/// @brief Name of the class.
		std::string name;

		/// @brief Functions exposed by the class.
		std::vector<LuaFunctionDefinition> functions;

		/// @brief Properties exposed by the class.
		std::vector<LuaParameterDefinition> properties;
	};

	/// @brief Describes an enum exposed through the Lua API.
	/// Contains the name of the enum and the flags that are available
	/// through the generated Lua API definition.
	struct LuaEnumDefinition
	{
		/// @brief Name of the enum.
		std::string name;

		std::vector<LuaFlagDefinition> flags;
	};

	/// @brief Describes a global Lua function.
	/// Contains the function name, return type, and parameters required to
	/// generate the corresponding global function declaration.
	struct LuaGlobalFunctionDefinition
	{
		/// @brief Name of the global function.
		std::string name;

		/// @brief Return type of the function.
		std::string returnType;

		/// @brief Parameters accepted by the function.
		std::vector<LuaParameterDefinition> parameters;
	};

	/// @brief Generates Lua API definition files from C++ API descriptions.
	/// LuaApiGenerator converts structured descriptions of Lua classes and
	/// global functions into a generated Lua API definition file. The generated
	/// definitions can be used by Lua tooling such as language servers to provide
	/// type information, function signatures, and code completion.
	class LuaApiGenerator
	{
	public:
		/// @brief Generates a Lua API definition file.
		/// Generates declarations for the provided global functions and classes
		/// and writes them to the specified output file.
		/// @param p_outputPath Path where the generated Lua API file should be written.
		/// @param p_globals Global Lua functions to include in the generated API.
		/// @param p_classes Lua-exposed classes to include in the generated API.
		/// @return True if the API file was generated successfully, otherwise false.
		static bool Generate(
			const std::filesystem::path &p_outputPath,
			const std::vector<LuaGlobalFunctionDefinition> &p_globals,
			const std::vector<LuaClassDefinition> &p_classes,
			const std::vector<LuaEnumDefinition> &p_enums
		);

	private:
		/// @brief Generates declarations for global Lua functions.
		/// Writes the provided global function definitions to the output file.
		/// @param p_file Output file to which the declarations are written.
		/// @param p_global Global function definitions to generate.
		static void GenerateGlobal(
			std::ofstream &p_file,
			const std::vector<LuaGlobalFunctionDefinition> p_global
		);

		/// @brief Generates declarations for Lua-exposed classes.
		/// Writes the provided class definitions and their functions to the output file.
		/// @param p_file Output file to which the declarations are written.
		/// @param p_classes Class definitions to generate.
		static void GenerateClasses(
			std::ofstream &p_file,
			const std::vector<LuaClassDefinition> &p_classes,
			const std::vector<LuaEnumDefinition> &p_enums
		);

		/// @brief Gets a default Lua value for a given type.
		/// Used when generating Lua API definitions that require a representative
		/// default value for a particular type.
		/// @param p_type Type for which a default Lua value should be generated.
		/// @return String representation of the default Lua value for the type.
		[[nodiscard]] static std::string GetDefaultLuaValue(const std::string &p_type);
	};
};
