/**
* @file SandboxShaderCompiler.hpp
 * @author Sebastian Lorensson
 * @date 2026-10-20
 * @brief Slang shader compiler for the sandbox environment.
 */

#pragma once

#include <string>
#include <vector>
#include <slang/slang-com-ptr.h>


namespace std::filesystem
{
    class path;
}
namespace slang
{
    struct IGlobalSession;
    struct ISession;
}

namespace Droplet::Sandbox
{
    
/// Sandbox class for compiling .slang shader source into SPIR-V data
class SandboxShaderCompiler
{
public:

    /// Initializes a global session and one local session.
    SandboxShaderCompiler();

    /// Compile one Slang entry point into SPIR-V data
    [[nodiscard]]
    std::vector<std::uint32_t> CompileSpirv(
        const char* moduleName,
        const std::filesystem::path& sourcePath,
        const char* entryPointName) const;
private:
    /**
     * @brief Builds a string of detail from a shader compilation diagnostics blob
     * @param diagnostics outDiagnostics param passed into the getEntryPointCode func
     * @return string of the diagnostic
     */
    static std::string GetSlangDiagnostics(
        ISlangBlob* diagnostics);

    /**
     * @brief Checks the result of a slang operation and stores the result in a blob.
     * @param result Result to evaluate
     * @param message Message for if the operation failed
     * @param diagnostics blob containing diagnostics data
     * @throw runtime_error with details if failed
     */
    static void CheckSlangResult(
        SlangResult result,
        const char* message,
        ISlangBlob* diagnostics = nullptr);

    /**
     * @brief Creates global session.
     * @throws runtime_error if creation failed
     */
    void CreateGlobalSession();

    /**
     * @brief creates a local session.
     * @throws runtime_error if creation failed
     */
    void CreateSession();
private:
    Slang::ComPtr<slang::IGlobalSession> m_globalSession; ///< Only call from main thread
    Slang::ComPtr<slang::ISession> m_session; ///< Compilation scope
};
}
    