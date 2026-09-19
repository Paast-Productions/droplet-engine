#include "ShaderCompiler.hpp"

#include <print>
#include <array>

using namespace Droplet::Graphics;

ShaderCompiler::ShaderCompiler()
{
    slang::createGlobalSession(m_globalSession.writeRef());
}

Slang::ComPtr<slang::IBlob> ShaderCompiler::CompileShader(const std::filesystem::path& p_path, const SlangcSessionParameters& p_sessionParams )
{
    Slang::ComPtr<slang::ISession> session {};
    
    // Check if session already exists
    if (m_sessionMap.contains(p_sessionParams))
    {
        session = m_sessionMap[p_sessionParams]; 
    }
    else
    {
        slang::SessionDesc sessionDesc 
        {  
            .targets = p_sessionParams.TargetDescriptions.data(),
            .targetCount = static_cast<SlangInt>(p_sessionParams.TargetDescriptions.size()),
            .defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
            .preprocessorMacros = p_sessionParams.PreprocessorMacroDescriptions.data(),
            .preprocessorMacroCount = static_cast<SlangInt>(p_sessionParams.PreprocessorMacroDescriptions.size()),
            .compilerOptionEntries = p_sessionParams.CompilerOptionEntries.data(),
            .compilerOptionEntryCount = static_cast<std::uint32_t>(p_sessionParams.CompilerOptionEntries.size()),
        };
        
        m_globalSession->createSession(sessionDesc, session.writeRef());
        
        m_sessionMap.insert(std::make_pair<const SlangcSessionParameters&, const Slang::ComPtr<slang::ISession>&>(p_sessionParams, session));
    }
    
    Slang::ComPtr<slang::IModule> module {};
    
    {
        Slang::ComPtr<slang::IBlob> errorBlob {};
        module = session->loadModule(p_path.filename().string().c_str(), errorBlob.writeRef());
        
        if (!module)
        {
            std::print("Shaderc Log:\n{0}", errorBlob->getBufferPointer());
            
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
            session->createCompositeComponentType
            (
                moduleComponent.data(),
                moduleComponent.size(),
                composedProgram.writeRef(),
                errorBlob.writeRef()
            )
        };
        
        if (result == SLANG_FAIL)
        {
            std::print("Shaderc log:\n{0}", static_cast<const char*>(errorBlob->getBufferPointer()));
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
            std::print("Shaderc log:\n{0}", static_cast<const char*>(errorBlob->getBufferPointer()));
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
            std::print("Shaderc log:\n{0}", static_cast<const char*>(errorBlob->getBufferPointer()));
            throw std::runtime_error("Failed to compile shader to SPIR-V");
        }
    }
    
    return spirvCode;
}
