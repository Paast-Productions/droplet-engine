#pragma once

#include <SceneSystem/Component.hpp>
#include <resource/types/MeshResource.hpp>
#include <SceneSystem/bounds/MeshNodeBounds.hpp>
#include <string>

namespace Droplet::Scene
{
    /// @brief Component that attaches a mesh to a Node.
    /// 
    /// an example of a MeshComponent could be a 3D model of a character, an object, or an environment element.
    /// 
    class MeshComponent : public Component
    {
    public:
        explicit MeshComponent(const std::string &p_meshPath);

        void Update(float p_deltaTime) override;

        /// @brief Gets the type name of the component.
        /// @return The type name of the component.
		std::string_view GetTypeName() override { return "MeshComponent"; }

        /// TODO: Placeholder as the correct method is not yet inplemented
        const std::string &GetMeshPath() const;

		/// @brief Gets the mesh resource associated with this component.
		/// @return A weak pointer to the mesh resource, or an empty weak pointer if the resource is not loaded.
		const std::weak_ptr<MeshResource> GetMeshResource() const;

        nlohmann::json SerializeImpl() override;
        void DeserializeImpl(nlohmann::json p_compJson) override;

        /// @brief Determines whether the component provides custom bounds.
        /// @return True if the component provides custom bounds, otherwise false.
		bool HasBoundsOverride() const override { return true; }

        /// @brief Gets the custom bounds provided by the component.
        /// @return The bounding box provided by the component.
		[[nodiscard]] std::shared_ptr<NodeBounds> GetBounds() const override { return std::make_shared<MeshNodeBounds>(m_bounds); }

    private:
        std::string m_meshPath;

        MeshNodeBounds m_bounds;
    };
}