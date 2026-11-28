#pragma once

#include <cstdint>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <Component.hpp>
#include <SceneSystem/Scene.hpp>

namespace Droplet::Scene
{
    /// @brief Steers a list of fish nodes with boids rules and keeps them clear of scenery.
    ///
    /// Each update every fish is steered by separation, alignment and cohesion with its neighbours,
    /// plus an avoidance steer when raycasts ahead hit scenery within avoidanceDistance.
    /// The fish and their child nodes are excluded from the raycasts.
    class BoidsComponent : public Component
    {
    public:
        BoidsComponent() = default;

        std::string_view GetTypeName() const override { return "BoidsComponent"; }

        /// @brief Adds a node to the flock. Adding a node that is already in the flock does nothing.
        void AddFish(const std::shared_ptr<Node> &p_fish);

        /// @brief Removes a node from the flock.
        void RemoveFish(const std::shared_ptr<Node> &p_fish);

        /// @return The fish in the flock whose nodes still exist.
        [[nodiscard]] std::vector<std::shared_ptr<Node>> GetFish() const;

    protected:
        void UpdateImpl() override;

        void ResolveReferences(SerializationContext &p_context) override;

        void RenderUIImpl() override;

        [[nodiscard]] nlohmann::json SerializeImpl() override;

        void DeserializeImpl(const nlohmann::json p_compJson) override;

    private:
        /// Editor slots, so entries can be empty while the user is still filling the list.
        std::vector<std::weak_ptr<Node>> m_fish;

        /// Serialized fish IDs that were not found in the scene when it loaded. Kept so saving does not drop them.
        std::vector<std::uint64_t> m_unresolvedFishIDs;

        /// Keyed by node so velocities stay with their fish when the editor reorders or removes entries.
        std::unordered_map<const Node *, glm::vec3> m_velocities;

        float m_perceptionRadius = 4.0f;
        float m_separationRadius = 1.5f;
        float m_separationWeight = 1.5f;
        float m_alignmentWeight = 1.0f;
        float m_cohesionWeight = 1.0f;
        float m_minSpeed = 1.0f;
        float m_maxSpeed = 3.0f;
        float m_maxSteeringForce = 6.0f;
        float m_avoidanceDistance = 3.0f;
        float m_avoidanceWeight = 4.0f;
    };
}
