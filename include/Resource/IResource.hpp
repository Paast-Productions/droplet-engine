#pragma once

namespace Droplet
{
    enum class ResourceType
    {
        None,
        Texture2D,
        Texture3D,
        Mesh,
        SkinnedMesh,
        Animation,
        Shader,
        Material,
        Sound
    };
    
    /// @brief Interface for all resources that can be loaded by the AssetManager. All resources must implement this interface.
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
