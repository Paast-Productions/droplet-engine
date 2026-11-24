#pragma once

#ifdef _WIN32
#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#elifdef __linux__
#include <shader-slang/slang.h>
#include <shader-slang/slang-com-ptr.h>
#endif

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace Droplet::IO
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
		Slang::ComPtr<slang::IBlob> CompileShader(const std::filesystem::path &p_path) const;

		/**
		 * @brief Compiles one entry point of a .slang file to SPIR-V (the renderer's built-in shaders)
		 * @param p_moduleName The Slang module name; compiled once per compiler, then reused
		 * @param p_sourcePath Path to the .slang file
		 * @param p_entryPointName The entry point, kept as the SPIR-V entry point's name
		 * @returns The SPIR-V words
		 */
		std::vector<std::uint32_t> CompileSpirv(
			const char *p_moduleName, const std::filesystem::path &p_sourcePath, const char *p_entryPointName) const;
	
	private:
		/// @brief The session CompileSpirv uses, created on first use
		slang::ISession &SpirvSession() const;

		/// @brief Throws with `p_message` and the diagnostics if `p_result` failed
		static void CheckSlangResult(SlangResult p_result, const char *p_message, ISlangBlob *p_diagnostics = nullptr);

		Slang::ComPtr<slang::IGlobalSession> m_globalSession {};
		mutable Slang::ComPtr<slang::ISession> m_spirvSession {}; ///< keeps compiled modules for the next entry point
	};
}
