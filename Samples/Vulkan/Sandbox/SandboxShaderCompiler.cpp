/**
* @file SandboxShaderCompiler.cpp
 * @author Sebastian Lorensson
 * @date 2026-10-20
 * @brief Source implementation for the sandbox environment Slang shader compiler.
 */
#include "SandboxShaderCompiler.hpp"

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>

#include "Core/FileUtils.hpp"

#include <filesystem>
#include <print>
#include <array>
#include <string>

using namespace Droplet::Sandbox;
// =====================================================================
// Slang compiler handler
// =====================================================================
SandboxShaderCompiler::SandboxShaderCompiler()
{
    CreateGlobalSession();
    CreateSession();
}

std::vector<std::uint32_t> SandboxShaderCompiler::CompileSpirv(
    const char* moduleName,
    const std::filesystem::path& sourcePath,
    const char* entryPointName) const
{
    if (moduleName == nullptr)
        throw std::invalid_argument("moduleName param was nullptr");

    if (entryPointName == nullptr)
        throw std::invalid_argument("entryPointName param was nullptr");

    std::vector<char> bytes = Core::Utils::ReadFileAsBytes(sourcePath);
    const std::string source{ bytes.begin(), bytes.end() };
    const std::string path = sourcePath.string();

    Slang::ComPtr<ISlangBlob> moduleDiagnostics;

#if 0
    // TODO: this does not work with slang v2026.17, needs a later version.
    // Slang::ComPtr<slang::IModule> module{
    // 	m_session->loadModuleFromSource(
    // 		moduleName,
    // 		path.c_str(),
    // 		nullptr,
    // 		moduleDiagnostics.writeRef())
    // };
#endif

    Slang::ComPtr<slang::IModule> module{
        m_session->loadModuleFromSourceString(
            moduleName,
            path.c_str(),
            source.c_str(),
            moduleDiagnostics.writeRef()
        )
    };

    if (!module)
    {
        throw std::runtime_error(
            "Failed to load Slang module '" +
            std::string(moduleName) + "': " +
            GetSlangDiagnostics(moduleDiagnostics.get()));
    }

    // Find the requested shader entry point.
    Slang::ComPtr<slang::IEntryPoint> entryPoint;

    const SlangResult entryPointResult =
        module->findEntryPointByName(
            entryPointName,
            entryPoint.writeRef());

    CheckSlangResult(
        entryPointResult,
        "Failed to find shader entry point");

    if (!entryPoint)
    {
        throw std::runtime_error(
            "Slang entry point not found: " +
            std::string(entryPointName));
    }

    // Compose the module with its selected entry point.
    const std::array<slang::IComponentType*, 2> components{
        module.get(),
        entryPoint.get()
    };

    Slang::ComPtr<slang::IComponentType> program;
    Slang::ComPtr<ISlangBlob> composeDiagnostics;

    const SlangResult composeResult =
        m_session->createCompositeComponentType(
            components.data(),
            static_cast<SlangInt>(components.size()),
            program.writeRef(),
            composeDiagnostics.writeRef());

    CheckSlangResult(
        composeResult,
        "Failed to compose Slang program",
        composeDiagnostics.get());

    // Link the program.
    Slang::ComPtr<slang::IComponentType> linkedProgram;
    Slang::ComPtr<ISlangBlob> linkDiagnostics;

    const SlangResult linkResult =
        program->link(
            linkedProgram.writeRef(),
            linkDiagnostics.writeRef());

    CheckSlangResult(
        linkResult,
        "Failed to link Slang program",
        linkDiagnostics.get());

    // Generate SPIR-V for the single selected entry point.
    Slang::ComPtr<ISlangBlob> spirv;
    Slang::ComPtr<ISlangBlob> compileDiagnostics;

    const SlangResult compileResult =
        linkedProgram->getEntryPointCode(
            0, // Entry point index in the composed program
            0, // Target index: SPIR-V
            spirv.writeRef(),
            compileDiagnostics.writeRef());

    CheckSlangResult(
        compileResult,
        "Failed to compile Slang entry point",
        compileDiagnostics.get());

    if (!spirv || spirv->getBufferSize() == 0)
    {
        throw std::runtime_error(
            "Slang produced empty SPIR-V bytecode.");
    }

    const std::size_t codeSize = spirv->getBufferSize();

    if (codeSize % sizeof(std::uint32_t) != 0)
    {
        throw std::runtime_error(
            "Invalid SPIR-V bytecode size.");
    }

    // Return owned, aligned SPIR-V words.
    std::vector<std::uint32_t> code(
        codeSize / sizeof(std::uint32_t));

    std::memcpy(
        code.data(),
        spirv->getBufferPointer(),
        codeSize);

    return code;
}

void SandboxShaderCompiler::CreateGlobalSession()
{
    const SlangResult result =
        slang::createGlobalSession(
            m_globalSession.writeRef());

    CheckSlangResult(
        result,
        "Failed to create Slang global session");

    std::println("Slang build: {}", m_globalSession->getBuildTagString());
}

void SandboxShaderCompiler::CreateSession()
{
    const std::array targets{
        slang::TargetDesc{
            .format = SLANG_SPIRV,
            .profile =
            m_globalSession->findProfile("spirv_1_4")
        }
    };

    const std::array options
    {
        slang::CompilerOptionEntry
        {
            .name = slang::CompilerOptionName::EmitSpirvDirectly,
            .value =
            {
                .kind = slang::CompilerOptionValueKind::Int,
                .intValue0 = 1
            }
        },
        slang::CompilerOptionEntry
        {
            .name = slang::CompilerOptionName::VulkanUseEntryPointName,
            .value =
            {
                .kind = slang::CompilerOptionValueKind::Int,
                .intValue0 = 1
            }
        }
    };

    const slang::SessionDesc sessionDesc
    {
        .targets = targets.data(),
        .targetCount =
        static_cast<SlangInt>(targets.size()),
        .defaultMatrixLayoutMode =
        SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
        .compilerOptionEntries = options.data(),
        .compilerOptionEntryCount =
        static_cast<std::uint32_t>(options.size())
    };

    const SlangResult result =
        m_globalSession->createSession(
            sessionDesc,
            m_session.writeRef());

    CheckSlangResult(
        result,
        "Failed to create Slang session");
}

std::string SandboxShaderCompiler::GetSlangDiagnostics(ISlangBlob* diagnostics)
{
    if (!diagnostics || !diagnostics->getBufferPointer())
        return {};

    const auto* text = static_cast<const char*>(
        diagnostics->getBufferPointer());

    std::string message(
        text, diagnostics->getBufferSize());

    // Diagnostics may include a terminating null byte.
    while (!message.empty() && message.back() == '\0')
        message.pop_back();

    return message;
}

void SandboxShaderCompiler::CheckSlangResult(
    SlangResult result,
    const char* message,
    ISlangBlob* diagnostics)
{
    if (SLANG_SUCCEEDED(result))
        return;

    std::string err{message};
    const std::string details = GetSlangDiagnostics(diagnostics);

    if (!details.empty())
    {
        err += ": ";
        err += details;
    }

    throw std::runtime_error(err);
}
