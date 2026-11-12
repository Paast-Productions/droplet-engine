#pragma once

namespace Droplet
{
    
    class IResource
    {
    public:
    
        IResource() = default;
        IResource(const IResource &p_other) = default;
        IResource &operator=(const IResource &p_other) = default;
        IResource(IResource &&p_other) noexcept = default;
        IResource &operator=(IResource &&p_other) noexcept = default;

        virtual ~IResource() = default;
        
    protected:
        
    private:
    
    };
    
}
