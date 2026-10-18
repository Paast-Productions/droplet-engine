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
        
        void GetRenderData(Graphics::RenderData &p_renderData) override;

        /// @brief Gets the type name of the component.
        /// @return The type name of the component.
        [[nodiscard]] std::string_view GetTypeName() const override { return "MeshComponent"; }

        // Mesh

        /// @brief Sets the mesh resource using its GUID.
        /// @param p_guid GUID identifying the mesh resource to load.
        void SetMeshGUID(Droplet::GUID p_guid) { m_mesh = ResourceManager::GetInstance().LoadResource<Droplet::MeshResource>(p_guid); } 

        /// @brief Gets the GUID of the assigned mesh resource.
        /// @return GUID of the assigned mesh resource, or the invalid GUID if none is assigned.
        [[nodiscard]] Droplet::GUID GetMeshGUID() const { return m_mesh.GetGUID(); }

        // Texture

        /// @brief Sets the 2D texture resource using its GUID.
        /// @param p_guid GUID identifying the texture resource to load.
        void SetTextureGUID(Droplet::GUID p_guid) { m_2dTexture = ResourceManager::GetInstance().LoadResource<Droplet::Texture2DResource>(p_guid); }

        /// @brief Gets the GUID of the assigned 2D texture resource.
        /// @return GUID of the assigned texture resource, or the invalid GUID if none is assigned.
        [[nodiscard]] Droplet::GUID GetTextureGUID() const { return m_2dTexture.GetGUID(); }
        
        void SetVSGUID(Droplet::GUID p_guid) { m_shaderData.vs = ResourceManager::GetInstance().LoadResource<ShaderResource>(p_guid); }
        [[nodiscard]] Droplet::GUID GetVSGUID() const { return m_shaderData.vs.GetGUID(); }
        
        void SetTCSGUID(Droplet::GUID p_guid) { m_shaderData.tcs = ResourceManager::GetInstance().LoadResource<ShaderResource>(p_guid); }
        [[nodiscard]] Droplet::GUID GetTCSGUID() const { return m_shaderData.tcs.GetGUID(); }
        
        void SetTESGUID(Droplet::GUID p_guid) { m_shaderData.tes = ResourceManager::GetInstance().LoadResource<ShaderResource>(p_guid); }
        [[nodiscard]] Droplet::GUID GetTESGUID() const { return m_shaderData.tes.GetGUID(); }
        
        void SetGSGUID(Droplet::GUID p_guid) { m_shaderData.gs = ResourceManager::GetInstance().LoadResource<ShaderResource>(p_guid); }
        [[nodiscard]] Droplet::GUID GetGSGUID() const { return m_shaderData.gs.GetGUID(); }
        
        void SetFSGUID(Droplet::GUID p_guid) { m_shaderData.fs = ResourceManager::GetInstance().LoadResource<ShaderResource>(p_guid); }
        [[nodiscard]] Droplet::GUID GetFSGUID() const { return m_shaderData.fs.GetGUID(); }
        
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

        ResourceHandle<MeshResource> m_mesh {};
        ResourceHandle<Texture2DResource> m_2dTexture {};

        // TODO: REWRITE AS MATERIAL
        struct ShaderResources
        {
            ResourceHandle<ShaderResource> vs {};
            ResourceHandle<ShaderResource> tcs {};
            ResourceHandle<ShaderResource> tes {};
            ResourceHandle<ShaderResource> gs {};
            ResourceHandle<ShaderResource> fs {};
        } m_shaderData;

        MeshNodeBounds m_bounds;
    };
}