#pragma once

#include <SceneSystem/Component.hpp>
#include <SceneSystem/bounds/MeshNodeBounds.hpp>
#include <resource/ResourceHandle.hpp>
#include <resource/types/MaterialResource.hpp>
#include <resource/types/MeshResource.hpp>
#include <resource/types/Texture2DResource.hpp>

#include <string>
#include <string_view>
#include <cstdint>

namespace Droplet::Scene
{
    /// @brief Component that attaches a mesh to a Node.
    /// 
    /// an example of a MeshComponent could be a 3D model of a character, an object, or an environment element.
    /// 
    class MeshComponent : public Component
    {
    public:
        MeshComponent();

        void Update(float p_deltaTime) override;

        /// @brief Gets the type name of the component.
        /// @return The type name of the component.
        [[nodiscard]] std::string_view GetTypeName() const override { return "MeshComponent"; }

        // Mesh

        /// @brief Sets the mesh resource using its GUID.
        /// @param p_guid GUID identifying the mesh resource to load.
        /// @param p_resourceManager Resource manager used to load the mesh resource.
        void SetMeshGUID(Droplet::GUID p_guid, Droplet::ResourceManager &p_resourceManager);

        /// @brief Gets the GUID of the assigned mesh resource.
        /// @return GUID of the assigned mesh resource, or the invalid GUID if none is assigned.
        [[nodiscard]] Droplet::GUID GetMeshGUID() const
        {
            return m_mesh.GetGUID();
        }

        // Texture

        /// @brief Sets the 2D texture resource using its GUID.
        /// @param p_guid GUID identifying the texture resource to load.
        /// @param p_resourceManager Resource manager used to load the texture resource.
        void SetTextureGUID(Droplet::GUID p_guid, Droplet::ResourceManager &p_resourceManager);

        /// @brief Gets the GUID of the assigned 2D texture resource.
        /// @return GUID of the assigned texture resource, or the invalid GUID if none is assigned.
        [[nodiscard]] Droplet::GUID GetTextureGUID() const
        {
            return m_2dTexture.GetGUID();
        }

        /// @brief Gets the resource handle for the assigned mesh.
        /// @return Constant reference to the mesh resource handle.
        [[nodiscard]] const Droplet::ResourceHandle<Droplet::MeshResource> &
            GetMeshResource() const;

        /// @brief Gets the resource handle for the assigned 2D texture.
        /// @return Constant reference to the 2D texture resource handle.
        [[nodiscard]] const Droplet::ResourceHandle<Droplet::Texture2DResource> &
            Get2dTextureResource() const;

        /// @brief Determines whether the component provides custom bounds.
        /// @return True if the component provides custom bounds, otherwise false.
        [[nodiscard]] bool HasBoundsOverride() const override { return true; }

        /// @brief Gets the custom bounds provided by the component.
        /// @return The bounding box provided by the component.
        [[nodiscard]] const NodeBounds *GetBounds() const override { return &m_bounds; }

    protected:
        void RenderUIImpl() override;

        nlohmann::json SerializeImpl() override;
        void DeserializeImpl(nlohmann::json p_compJson) override;

    private:

        // Resource handles

        ResourceHandle<MeshResource> m_mesh;
        ResourceHandle<Texture2DResource> m_2dTexture;

        MeshNodeBounds m_bounds;
    };
}