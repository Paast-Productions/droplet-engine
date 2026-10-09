#pragma once

#include <SceneSystem/Component.hpp>
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
        MeshComponent() = default;

        void Update(float p_deltaTime) override;

        /// @brief Gets the type name of the component.
        /// @return The type name of the component.
		std::string_view GetTypeName() override { return "MeshComponent"; }

        /// TODO: Placeholder as the correct method is not yet inplemented
        const std::string &GetMeshPath() const;

        nlohmann::json SerializeImpl() override;
        void DeserializeImpl(nlohmann::json p_compJson) override;
    private:
        std::string m_meshPath;
    };
}