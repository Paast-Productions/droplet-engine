#pragma once

#include <Resource/IDynamicResource.hpp>

namespace Droplet
{
    class RenderTargetResource : public IDynamicResource
    {
    public:
        RenderTargetResource() = default;
        RenderTargetResource(const RenderTargetResource &p_other) = default;
        RenderTargetResource &operator=(const RenderTargetResource &p_other) = default;
        RenderTargetResource(RenderTargetResource &&p_other) noexcept = default;
        RenderTargetResource &operator=(RenderTargetResource &&p_other) noexcept = default;

        ~RenderTargetResource() = default;
    
    };
}
