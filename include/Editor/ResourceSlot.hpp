#pragma once

#include <string>
#include <Resource/GuidUtils.hpp>
#include <Resource/IResource.hpp>

namespace Droplet
{
    bool ResourceSlot(
        const char *p_label,
        Droplet::GUID p_current,
        const std::string &p_currentDisplayName,
        Droplet::GUID &p_outGUID,
        std::string &p_outDisplayName,
        Droplet::ResourceType p_expectedType);
}
