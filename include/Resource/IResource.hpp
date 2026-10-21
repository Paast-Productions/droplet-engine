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
    };
    
    /// @brief Interface for all resources that can be loaded by the AssetManager. All resources must implement this interface.
    class IResource
    {
    public:

    protected:

    private:

        // TODO: Declare
    };
}
