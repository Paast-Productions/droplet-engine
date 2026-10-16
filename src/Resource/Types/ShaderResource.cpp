#pragma once
#include "resource/types/ShaderResource.hpp"
#include <tracy/public/tracy/Tracy.hpp>

namespace Droplet
{
    ShaderResource::ShaderResource(ShaderType p_type, std::pair<Slang::ComPtr<slang::IBlob>, Slang::ComPtr<slang::ISession>> p_shaderByteCode)
        : m_type(p_type), m_byteCode(p_shaderByteCode.first), m_session(p_shaderByteCode.second) {}

    std::unique_ptr<ShaderResource> ShaderResource::CreateFallback()
    {
        ZoneScoped;

        return std::make_unique<ShaderResource>(ShaderType::Vertex, std::make_pair<Slang::ComPtr<slang::IBlob>, Slang::ComPtr<slang::ISession>>(nullptr, nullptr));
    }

    ShaderResource::ShaderType ShaderResource::GetType() const
    {
        return m_type;
    }

    Slang::ComPtr<slang::IBlob> ShaderResource::GetByteCode() const
    {
        return m_byteCode;
    }

    void ShaderResource::ReleaseRam()
    {
        m_byteCode = nullptr; // ComPtr is reference counted internally so this is safe
    }
}
