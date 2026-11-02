#pragma once

#include <string>
#include <Resource/GuidUtils.hpp>
#include <Resource/IResource.hpp>

#include "ShaderResource.hpp"

namespace Droplet
{
    bool ResourceSlot(
        const char *p_label,
        Droplet::GUID p_current,
        const std::string &p_currentDisplayName,
        Droplet::GUID &p_outGUID,
        std::string &p_outDisplayName,
        Droplet::ResourceType p_expectedType,
        Droplet::ShaderResource::ShaderType p_expectedShader = Droplet::ShaderResource::ShaderType::Vertex);
}
