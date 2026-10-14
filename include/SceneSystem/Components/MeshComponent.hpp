#pragma once

#include <SceneSystem/Component.hpp>
#include <resource/types/MeshResource.hpp>
#include <SceneSystem/bounds/MeshNodeBounds.hpp>
#include <resource/ResourceHandle.hpp>
#include <resource/types/MaterialResource.hpp>

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
		std::string_view GetTypeName() const override { return "MeshComponent"; }

        // Mesh

        //TODO: should be GUID or something idk
        void SetMeshGUID(std::uint64_t p_meshGUID);
        [[nodiscard]] std::uint64_t GetMeshGUID() const;

        // Texture
        // 
         //TODO: should be GUID or something idk
        void SetTextureGUID(std::uint64_t p_textureGUID);
        [[nodiscard]] std::uint64_t GetTextureGUID() const;

		/// @brief Gets the mesh resource associated with this component.
		/// @return A weak pointer to the mesh resource, or an empty weak pointer if the resource is not loaded.
		const std::weak_ptr<MeshResource> GetMeshResource() const;

        /// @brief Determines whether the component provides custom bounds.
        /// @return True if the component provides custom bounds, otherwise false.
        bool HasBoundsOverride() const override { return true; }

        /// @brief Gets the custom bounds provided by the component.
        /// @return The bounding box provided by the component.
        [[nodiscard]] std::shared_ptr<NodeBounds> GetBounds() const override { return std::make_shared<MeshNodeBounds>(m_bounds); }


		/// @brief Determines whether the component makes the Node renderable.
		/// @return true if the component is renderable, otherwise false.
        bool IsRenderable() const override
        {
            return true;
        }

    protected:
		void RenderUIImpl() override;

        nlohmann::json SerializeImpl() override;
        void DeserializeImpl(nlohmann::json p_compJson) override;

    private:
        std::uint64_t m_meshGUID = 0;
        std::uint64_t m_textureGUID = 0;

        MeshNodeBounds m_bounds;

        ResourceHandle<MeshResource> m_mesh;
        ResourceHandle<MaterialResource> m_material;

        // TODO: Material settings
    };
}