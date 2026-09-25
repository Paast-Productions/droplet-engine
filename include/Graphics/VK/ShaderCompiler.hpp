#pragma once

#include <filesystem>
#include <vector>

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>

namespace Droplet::Graphics
{	
	/// @struct SlangcConfig
	/// @brief Slang Shader Compiler Options\n
	/// 
	/// Options:
	/// - std::vector<slang::TargetDesc>				TargetDescriptions [{}] 
	/// - std::vector<slang::PreprocessorMacroDesc>		PreprocessorMacroDescriptions [{}] 
	/// - std::vector<slang::CompilerOptionEntry>		CompilerOptionEntries [{}] 
	struct SlangcConfig
	{
		std::vector<slang::TargetDesc> TargetDescriptions {};
		std::vector<slang::PreprocessorMacroDesc> PreprocessorMacroDescriptions {};
		std::vector<slang::CompilerOptionEntry> CompilerOptionEntries {};
	};
	
	/// @class ShaderCompiler
	/// @brief Slang Shader Compiler
	class ShaderCompiler
	{
	public:
		ShaderCompiler();
		ShaderCompiler(const ShaderCompiler &) = delete;
		ShaderCompiler &operator=(const ShaderCompiler &) = delete;
		ShaderCompiler(ShaderCompiler &&) = delete;
		ShaderCompiler &operator=(ShaderCompiler &&) = delete;
		
		/// @brief Compile file from .slang shader into a Blob
		/// @param p_path Path to the slang shader
		/// @returns Com-pointer of compiled shader Blob
		Slang::ComPtr<slang::IBlob> CompileShader(const std::filesystem::path &p_path);
	
	private:
		Slang::ComPtr<slang::IGlobalSession> m_globalSession {};
	};
}
