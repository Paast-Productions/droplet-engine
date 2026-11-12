#pragma once
#ifdef _EDITOR

#include <string>
#include <Resource/GuidUtils.hpp>
#include <Resource/IStaticResource.hpp>

#include "ShaderResource.hpp"

namespace Droplet
{
    bool ResourceSlot(
        const char *p_label,
        Droplet::GUID p_current,
        const std::string &p_currentDisplayName,
        Droplet::GUID &p_outGUID,
        std::string &p_outDisplayName,
        Droplet::StaticResourceType p_expectedType,
        Droplet::ShaderResource::ShaderType p_expectedShader = Droplet::ShaderResource::ShaderType::Vertex);
}
#endif
