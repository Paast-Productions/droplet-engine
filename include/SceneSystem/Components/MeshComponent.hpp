#pragma once

#include <SceneSystem/Component.hpp>
#include <resource/types/MaterialResource.hpp>
#include <resource/types/MeshResource.hpp>
#include <SceneSystem/bounds/MeshNodeBounds.hpp>
#include <string>
#include <string_view>
#include <ResourceManager.hpp>

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

		/// @brief Gets the GUID of the mesh resource associated with this component.
		/// @return The GUID of the mesh resource.
		[[nodiscard]] GUID GetMeshGUID() const { return m_meshGUID; }

		/// @brief Gets the GUID of the material resource associated with this component.
		/// @return The GUID of the material resource.
		[[nodiscard]] GUID GetMaterialGUID() const { return m_materialGUID; }

		/// @brief Sets the GUID of the mesh resource associated with this component.
		/// @param p_meshGUID The GUID of the mesh resource.
		void SetMeshGUID(GUID p_meshGUID);

		/// @brief Sets the GUID of the material resource associated with this component.
		/// @param p_materialGUID The GUID of the material resource.
		void SetMaterialGUID(GUID p_materialGUID);

		/// @brief Gets the mesh resource associated with this component.
		/// @return A weak pointer to the mesh resource, or an empty weak pointer if the resource is not loaded.
        [[nodiscard]] const std::weak_ptr<MeshResource> GetMeshResource() const;

		/// @brief Gets the material resource associated with this component.
		/// @return A weak pointer to the material resource, or an empty weak pointer if the resource is not loaded.
        [[nodiscard]] const std::weak_ptr<MaterialResource> GetMaterialResource() const;

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
		GUID m_meshGUID = C_INVALID_GUID;
		GUID m_materialGUID = C_INVALID_GUID;

        MeshNodeBounds m_bounds;
    };
}