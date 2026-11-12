#pragma once

#include <Resource/IResource.hpp>

namespace Droplet
{
    enum class StaticResourceType
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
    class IStaticResource : public IResource
    {
    public:

    protected:

    private:

        // TODO: Declare
    };
}
