#pragma once

#include <cstdint>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <Component.hpp>
#include <SceneSystem/Scene.hpp>

namespace Droplet
{
    template <typename Payload>
    class DeferredTask;
}

namespace Droplet::Scene
{
    /// @brief How the flock is constrained.
    enum class BoidsMode
    {
        /// Fish are only steered by flocking and scenery avoidance.
        Free,
        /// Fish stay inside at least one of the bounds nodes' oriented boxes.
        Bounds,
        /// Fish loosely follow a closed loop through the path nodes' positions.
        Path
    };

    /// @brief Steers a list of fish nodes with boids rules and keeps them clear of scenery.
    ///
    /// Each update every fish is steered by separation, alignment and cohesion with its neighbours,
    /// plus an avoidance steer when raycasts ahead hit scenery within avoidanceDistance.
    /// The fish and their child nodes are excluded from the raycasts.
    ///
    /// In Bounds mode a bounds node's box is its world transform applied to a unit cube.
    /// In Path mode the path is the closed loop through the path nodes' world positions, in list order.
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

        void EditorUpdateImpl() override;

        void ResolveReferences(SerializationContext &p_context) override;

        void RenderUIImpl() override;

        [[nodiscard]] nlohmann::json SerializeImpl() override;

        void DeserializeImpl(const nlohmann::json p_compJson) override;

    private:
        struct Flock;
        struct SteeringForces;
        struct Tuning;
        struct SteeringJob;

        /// Collects the live fish, their velocities and the scenery to ignore for one update.
        [[nodiscard]] Flock GatherFlock();

        /// Copies the settings that the flocking math reads, so a job can run without touching the component.
        [[nodiscard]] Tuning MakeTuning() const;

        /// Fills p_forces with one entry per fish in p_flock, computed on this thread. Terms are weighted; total is clamped to m_maxSteeringForce.
        void ComputeSteering(Scene &p_scene, Flock &p_flock, std::vector<SteeringForces> &p_forces);

        /// Sets the avoidance term for each live fish. Raycasts run in parallel when m_multithreaded is set, after the transforms are warmed.
        void ComputeAvoidanceForces(Scene &p_scene, Flock &p_flock, std::vector<SteeringForces> &p_forces);

        /// Sets the flocking, bounds and path terms, then the clamped total that includes the avoidance term already in p_forces.
        /// Reads only its arguments, so it is safe to run on any thread. Runs in parallel when p_tuning.multithreaded is set.
        static void ComputeFlockingForces(const Tuning &p_tuning, const Flock &p_flock, std::vector<SteeringForces> &p_forces);

        /// Starts the flocking steer for p_flock on the thread pool. Avoidance is raycast on this thread first, since scenery must not change while rays run.
        [[nodiscard]] std::shared_ptr<DeferredTask<SteeringJob>> StartSteeringJob(Scene &p_scene, const Flock &p_flock);

        /// Moves each live fish in p_flock by the total steering found for it in p_source, which may be a snapshot from an earlier update.
        /// Fish missing from p_source get no steering. Positions and velocities come from p_flock.
        void ApplySteering(const Flock &p_flock, const Flock &p_source, const std::vector<SteeringForces> &p_sourceForces, float p_deltaTime);

        /// Draws the bounds boxes and the path loop.
        void DrawBoundsAndPath();

        /// Draws the cluster and steering data for the flock.
        void DrawSteeringDebug();

        /// Set while the inspector shows this component, so bounds and path are drawn in the editor.
        bool m_uiSelected{ false };

        /// Draws the steering data in the editor.
        bool m_debugInfo{ false };

        /// Runs the flocking steer on the engine thread pool. Its result is applied on the next update, so the fish steer one frame late.
        bool m_multithreaded{ false };

        /// Flocking steer started on the last update, applied on the next one.
        std::shared_ptr<DeferredTask<SteeringJob>> m_pendingJob;

        /// Editor slots, so entries can be empty while the user is still filling the list.
        std::vector<std::weak_ptr<Node>> m_fish;

        /// Serialized fish IDs that were not found in the scene when it loaded. Kept so saving does not drop them.
        std::vector<std::uint64_t> m_unresolvedFishIDs;

        BoidsMode m_mode = BoidsMode::Free;

        std::vector<std::weak_ptr<Node>> m_bounds;
        std::vector<std::uint64_t> m_unresolvedBoundIDs;

        std::vector<std::weak_ptr<Node>> m_path;
        std::vector<std::uint64_t> m_unresolvedPathIDs;

        /// Keyed by node so velocities stay with their fish when the editor reorders or removes entries.
        std::unordered_map<const Node *, glm::vec3> m_velocities;

        float m_boundsWeight = 4.0f;
        float m_boundsMargin = 1.0f;
        float m_pathWeight = 2.0f;
        float m_pathLookahead = 3.0f;

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

		Droplet::CollisionLayer m_avoidanceLayerMask = Droplet::CollisionLayer::All;
    };
}
