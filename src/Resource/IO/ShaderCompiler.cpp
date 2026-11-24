#include <Resource/IO/ShaderCompiler.hpp>

#include <array>
#include <cstring>
#include <fstream>
#include <iterator>
#include <print>
#include <stdexcept>

using namespace Droplet::IO;

ShaderCompiler::ShaderCompiler()
{
    SlangResult res = slang::createGlobalSession(m_globalSession.writeRef());
    if (SLANG_FAILED(res))
    {
        throw std::runtime_error("Failed to initialize Slang Global Session.");
    }
}

Slang::ComPtr<slang::IBlob> ShaderCompiler::CompileShader(const std::filesystem::path &p_path) const
{
    
    Slang::ComPtr<slang::ISession> localSession {};
    
    slang::TargetDesc targetDesc
    {
        .format = SLANG_SPIRV,
        .profile = m_globalSession->findProfile("spirv_1_4")
    };
    
    std::array<slang::CompilerOptionEntry, 1> options
    {
        {
            {
                slang::CompilerOptionName::EmitSpirvDirectly,
                {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}
            }
        }
    };
    
    // Tell slang which directory to search in
    std::string searchDirectory = p_path.parent_path().generic_string();
    const char* searchPaths[] = { searchDirectory.c_str() };
    
    slang::SessionDesc sessionDesc 
    {  
        .targets = &targetDesc,
        .targetCount = 1,
        .defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
        .searchPaths = searchPaths,
        .searchPathCount = 1,
        .compilerOptionEntries = options.data(),
        .compilerOptionEntryCount = static_cast<std::uint32_t>(options.size()) 
    };
    
    m_globalSession->createSession(sessionDesc, localSession.writeRef());
    
    Slang::ComPtr<slang::IModule> module {};
    
    {
        Slang::ComPtr<slang::IBlob> errorBlob {};
        std::string moduleName = p_path.stem().string();
        module = localSession->loadModule(moduleName.c_str(), errorBlob.writeRef());
        
        if (!module)
        {
            // TODO: (LOGGING) Use logger for this
            if (errorBlob)
            {
                std::print("Shaderc Log:\n{0}", static_cast<const char*>(errorBlob->getBufferPointer()));
            }
            
            throw std::runtime_error("Failed to create slang module");
        }
    }
    
    std::array<slang::IComponentType*, 1> moduleComponent
    {
        {module}
    };
    
    Slang::ComPtr<slang::IComponentType> composedProgram {};
    
    {
        Slang::ComPtr<slang::IBlob> errorBlob {};
        SlangResult result
        {
            localSession->createCompositeComponentType
            (
                moduleComponent.data(),
                moduleComponent.size(),
                composedProgram.writeRef(),
                errorBlob.writeRef()
            )
        };
        
        if (result == SLANG_FAIL)
        {
            // TODO: (LOGGING) Use logger for this
            if (errorBlob)
            {
                std::print("Shaderc log:\n{0}", static_cast<const char*>(errorBlob->getBufferPointer()));
            }
            throw std::runtime_error("Failed to compose shader program");
        }
    }
    
    Slang::ComPtr<slang::IComponentType> linkedProgram {};
    
    {
        Slang::ComPtr<slang::IBlob> errorBlob {};
        SlangResult result 
        {  
            composedProgram->link
            (
                linkedProgram.writeRef(),
                errorBlob.writeRef()
            )
        };
        
        if (result == SLANG_FAIL)
        {
            // TODO: (LOGGING) Use logger for this
            if (errorBlob)
            {
                std::print("Shaderc log:\n{0}", static_cast<const char*>(errorBlob->getBufferPointer()));
            }
            throw std::runtime_error("Failed to link shader program");
        }
    }
    
    Slang::ComPtr<slang::IBlob> spirvCode {};
    
    {
        Slang::ComPtr<slang::IBlob> errorBlob {};
        SlangResult result
        {
            linkedProgram->getTargetCode
            (
                0,
                spirvCode.writeRef(),
                errorBlob.writeRef()
            )
        };
        
        if (result == SLANG_FAIL)
        {
            // TODO: (LOGGING) Use logger for this
            if (errorBlob)
            {
                std::print("Shaderc log:\n{0}", static_cast<const char*>(errorBlob->getBufferPointer()));
            }
            throw std::runtime_error("Failed to compile shader to SPIR-V");
        }
    }
    
    return spirvCode;
}

std::vector<std::uint32_t> ShaderCompiler::CompileSpirv(
    const char *p_moduleName, const std::filesystem::path &p_sourcePath, const char *p_entryPointName) const
{
    if (!p_moduleName || !p_entryPointName)
    {
        throw std::invalid_argument("CompileSpirv: no module or entry point name");
    }

    std::ifstream file(p_sourcePath, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Failed to open file:" + p_sourcePath.string());
    }
    const std::string source{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
    const std::string path = p_sourcePath.string();

    slang::ISession &session = SpirvSession();

    Slang::ComPtr<ISlangBlob> moduleDiagnostics {};
    Slang::ComPtr<slang::IModule> module {
        session.loadModuleFromSourceString(p_moduleName, path.c_str(), source.c_str(), moduleDiagnostics.writeRef())
    };
    if (!module)
    {
        CheckSlangResult(SLANG_FAIL, ("Failed to load Slang module '" + std::string(p_moduleName) + "'").c_str(),
                         moduleDiagnostics.get());
    }

    Slang::ComPtr<slang::IEntryPoint> entryPoint {};
    CheckSlangResult(module->findEntryPointByName(p_entryPointName, entryPoint.writeRef()),
                     "Failed to find shader entry point");
    if (!entryPoint)
    {
        throw std::runtime_error("Slang entry point not found: " + std::string(p_entryPointName));
    }

    // the module with its one entry point, linked
    const std::array<slang::IComponentType *, 2> components { module.get(), entryPoint.get() };
    Slang::ComPtr<slang::IComponentType> program {};
    Slang::ComPtr<ISlangBlob> composeDiagnostics {};
    CheckSlangResult(session.createCompositeComponentType(components.data(), static_cast<SlangInt>(components.size()),
                                                          program.writeRef(), composeDiagnostics.writeRef()),
                     "Failed to compose Slang program", composeDiagnostics.get());

    Slang::ComPtr<slang::IComponentType> linkedProgram {};
    Slang::ComPtr<ISlangBlob> linkDiagnostics {};
    CheckSlangResult(program->link(linkedProgram.writeRef(), linkDiagnostics.writeRef()),
                     "Failed to link Slang program", linkDiagnostics.get());

    Slang::ComPtr<ISlangBlob> spirv {};
    Slang::ComPtr<ISlangBlob> compileDiagnostics {};
    CheckSlangResult(linkedProgram->getEntryPointCode(0, 0, spirv.writeRef(), compileDiagnostics.writeRef()),
                     "Failed to compile Slang entry point", compileDiagnostics.get());

    const std::size_t codeSize = spirv ? spirv->getBufferSize() : 0;
    if (codeSize == 0 || codeSize % sizeof(std::uint32_t) != 0)
    {
        throw std::runtime_error("Slang produced no or malformed SPIR-V");
    }
    std::vector<std::uint32_t> code(codeSize / sizeof(std::uint32_t));
    std::memcpy(code.data(), spirv->getBufferPointer(), codeSize);
    return code;
}

slang::ISession &ShaderCompiler::SpirvSession() const
{
    if (m_spirvSession)
    {
        return *m_spirvSession;
    }

    const slang::TargetDesc target
    {
        .format = SLANG_SPIRV,
        .profile = m_globalSession->findProfile("spirv_1_4")
    };

    // entry points keep their names (vMain, fMain, cMain...) instead of becoming "main"
    std::array<slang::CompilerOptionEntry, 2> options {};
    options[0].name = slang::CompilerOptionName::EmitSpirvDirectly;
    options[0].value.kind = slang::CompilerOptionValueKind::Int;
    options[0].value.intValue0 = 1;
    options[1].name = slang::CompilerOptionName::VulkanUseEntryPointName;
    options[1].value.kind = slang::CompilerOptionValueKind::Int;
    options[1].value.intValue0 = 1;

    const slang::SessionDesc sessionDesc
    {
        .targets = &target,
        .targetCount = 1,
        .defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
        .compilerOptionEntries = options.data(),
        .compilerOptionEntryCount = static_cast<std::uint32_t>(options.size())
    };
    CheckSlangResult(m_globalSession->createSession(sessionDesc, m_spirvSession.writeRef()),
                     "Failed to create Slang session");
    return *m_spirvSession;
}

void ShaderCompiler::CheckSlangResult(SlangResult p_result, const char *p_message, ISlangBlob *p_diagnostics)
{
    if (SLANG_SUCCEEDED(p_result))
    {
        return;
    }

    std::string error { p_message };
    if (p_diagnostics && p_diagnostics->getBufferPointer())
    {
        std::string details(static_cast<const char *>(p_diagnostics->getBufferPointer()), p_diagnostics->getBufferSize());
        while (!details.empty() && details.back() == '\0')
        {
            details.pop_back();
        }
        if (!details.empty())
        {
            error += ": " + details;
        }
    }
    throw std::runtime_error(error);
}
