#pragma once

#include <Resource/IResource.hpp>

namespace Droplet
{
    
    class IDynamicResource : public IResource
    {
    public:
        IDynamicResource() = default;
        IDynamicResource(const IDynamicResource &p_other) = default;
        IDynamicResource &operator=(const IDynamicResource &p_other) = default;
        IDynamicResource(IDynamicResource &&p_other) noexcept = default;
        IDynamicResource &operator=(IDynamicResource &&p_other) noexcept = default;
    	
    	~IDynamicResource() override = default;
        
    protected:
        
    private:
        
    };
}
