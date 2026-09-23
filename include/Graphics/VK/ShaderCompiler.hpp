#pragma once

#include <filesystem>
#include <vector>

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>

namespace Droplet::Graphics
{
	struct SlangcSessionParameters
	{
		std::vector<slang::TargetDesc> TargetDescriptions {};
		std::vector<slang::PreprocessorMacroDesc> PreprocessorMacroDescriptions {};
		std::vector<slang::CompilerOptionEntry> CompilerOptionEntries {};
	};
	
	class ShaderCompiler
	{
	public:
		ShaderCompiler();
		ShaderCompiler(const ShaderCompiler&) = delete;
		ShaderCompiler& operator=(const ShaderCompiler&) = delete;
		ShaderCompiler(ShaderCompiler&&) = delete;
		ShaderCompiler& operator=(ShaderCompiler&&) = delete;
		
		Slang::ComPtr<slang::IBlob> CompileShader(const std::filesystem::path& p_path);
	
	private:
		Slang::ComPtr<slang::IGlobalSession> m_globalSession {};
	};
}
