#include "ShaderCompiler.hpp"

#include <print>
#include <array>

using namespace Droplet::Graphics;

ShaderCompiler::ShaderCompiler()
{
    SlangResult res = slang::createGlobalSession(m_globalSession.writeRef());
    if (SLANG_FAILED(res))
    {
        throw std::runtime_error("Failed to initialize Slang Global Session.");
    }
}

Slang::ComPtr<slang::IBlob> ShaderCompiler::CompileShader(const std::filesystem::path& p_path)
{
    Slang::ComPtr<slang::ISession> localSession{};
    
    slang::TargetDesc targetDesc
    {
        .format = SLANG_SPIRV,
        .profile = m_globalSession->findProfile("spirv_1_6")
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
    
    slang::SessionDesc sessionDesc 
    {  
        .targets = &targetDesc,
        .targetCount = 1,
        .defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
        .compilerOptionEntries = options.data(),
        .compilerOptionEntryCount = static_cast<std::uint32_t>(options.size()) 
    };
    
    m_globalSession->createSession(sessionDesc, localSession.writeRef());
    
    Slang::ComPtr<slang::IModule> module {};
    
    {
        Slang::ComPtr<slang::IBlob> errorBlob {};
        module = localSession->loadModule(p_path.filename().string().c_str(), errorBlob.writeRef());
        
        if (!module)
        {
            // TODO: Use logger for this
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
            // TODO: Use logger for this
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
            // TODO: Use logger for this
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
            // TODO: Use logger for this
            if (errorBlob)
            {
                std::print("Shaderc log:\n{0}", static_cast<const char*>(errorBlob->getBufferPointer()));
            }
            throw std::runtime_error("Failed to compile shader to SPIR-V");
        }
    }
    
    return spirvCode;
}
