#pragma once
#include "resource/types/ShaderResource.hpp"

namespace Droplet
{
    ShaderResource::ShaderResource(ShaderType p_type, Slang::ComPtr<slang::IBlob> p_byteCode)
        : m_type(p_type), m_byteCode(p_byteCode) {}

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
