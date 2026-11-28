#include "BoidsComponent.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

#include <ImGui/imgui.h>

#ifdef _EDITOR
#include <Editor/NodeSlot.hpp>
#endif

#include <Graphics/Debug/DebugDraw.hpp>
#include <Math/Bounds/OBB.hpp>
#include <Math/Bounds/Sphere.hpp>
#include <Node.hpp>
#include <Time.hpp>
#include <Transform.hpp>
#include <SceneSystem/Scene.hpp>
#include <SceneSystem/Serialization/SerializationContext.hpp>

using namespace Droplet::Scene;
using namespace Droplet::Math;

namespace
{
    constexpr float EPSILON = 1.0e-6f;
    constexpr float MAX_DELTA_TIME = 0.1f;
    constexpr float TAU = 6.2831853f;
    constexpr int AVOIDANCE_RAY_COUNT = 8;
    constexpr float AVOIDANCE_CONE_ANGLE = 0.6109f; // 35 degrees, in radians
    constexpr float DEBUG_FORCE_SCALE = 0.25f; ///< Length in world units per unit of steering force.
    constexpr float DEBUG_MARKER_RADIUS = 0.25f;

    template<typename T>
    void ReadValue(const nlohmann::json &p_json, const char *p_key, T &p_value)
    {
        if (p_json.contains(p_key))
        {
            p_value = p_json.at(p_key).get<T>();
        }
    }

    bool ContainsNode(const std::vector<std::weak_ptr<Node>> &p_nodes, const Node *p_node)
    {
        return std::ranges::any_of(p_nodes, [p_node](const std::weak_ptr<Node> &p_weak)
        {
            return p_weak.lock().get() == p_node;
        });
    }

    /// Returns a steer that turns the heading toward the side ray with the most free distance,
    /// or zero when the heading is clear within the range.
    glm::vec3 ComputeAvoidance(Scene &p_scene, const glm::vec3 &p_position, const glm::vec3 &p_heading,
        std::vector<std::weak_ptr<Node>> &p_ignoreNodes, float p_range, Droplet::CollisionLayer p_avoidanceLayerMask)
    {
        ZoneScoped;

        RayHit forwardHit;
        if (p_scene.RaycastNode(Ray(p_position, p_heading), &forwardHit, &p_ignoreNodes, p_avoidanceLayerMask, p_range).expired())
        {
            return glm::vec3(0.0f);
        }
        const float urgency = 1.0f - std::clamp(forwardHit.distance / p_range, 0.0f, 1.0f);

        const glm::vec3 reference = std::abs(p_heading.y) < 0.99f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
        const glm::vec3 right = glm::normalize(glm::cross(p_heading, reference));
        const glm::vec3 up = glm::cross(right, p_heading);

        glm::vec3 bestDirection = p_heading;
        float bestDistance = -1.0f;
        for (int i = 0; i < AVOIDANCE_RAY_COUNT; ++i)
        {
            const float angle = TAU * static_cast<float>(i) / static_cast<float>(AVOIDANCE_RAY_COUNT);
            const glm::vec3 lateral = right * std::cos(angle) + up * std::sin(angle);
            const glm::vec3 direction = glm::normalize(p_heading * std::cos(AVOIDANCE_CONE_ANGLE) + lateral * std::sin(AVOIDANCE_CONE_ANGLE));

            RayHit sideHit;
            const bool blocked = !p_scene.RaycastNode(Ray(p_position, direction), &sideHit, &p_ignoreNodes, p_avoidanceLayerMask, p_range).expired();
            const float freeDistance = blocked ? sideHit.distance : p_range;
            if (freeDistance > bestDistance)
            {
                bestDistance = freeDistance;
                bestDirection = direction;
            }
        }

        return (bestDirection - p_heading) * urgency;
    }

    constexpr float BOUNDS_TOLERANCE = 1.0e-4f;

    /// The node's world transform applied to a unit cube, so a node with scale 1 spans one unit.
    OBB GetNodeBox(Node &p_node)
    {
        return OBB::Transform(OBB(glm::vec3(0.0f), glm::vec3(0.5f)), p_node.GetTransform().GetMatrix(Transform::Space::World));
    }

    glm::vec3 ToBoxLocal(const OBB &p_box, const glm::vec3 &p_point)
    {
        return glm::transpose(p_box.orientation) * (p_point - p_box.center);
    }

    /// Distance from the point to the nearest face of the box: positive inside, negative outside.
    float BoxDepth(const OBB &p_box, const glm::vec3 &p_point)
    {
        const glm::vec3 margin = p_box.extents - glm::abs(ToBoxLocal(p_box, p_point));
        return std::min({margin.x, margin.y, margin.z});
    }

    glm::vec3 ClosestPointOnBox(const OBB &p_box, const glm::vec3 &p_point)
    {
        const glm::vec3 local = glm::clamp(ToBoxLocal(p_box, p_point), -p_box.extents, p_box.extents);
        return p_box.center + p_box.orientation * local;
    }

    /// Closest point on any of the boxes, or the point itself when there are no boxes.
    glm::vec3 ClosestPointOnBoxes(const std::vector<OBB> &p_boxes, const glm::vec3 &p_point)
    {
        glm::vec3 best = p_point;
        float bestDistSq = std::numeric_limits<float>::max();
        for (const OBB &box : p_boxes)
        {
            const glm::vec3 candidate = ClosestPointOnBox(box, p_point);
            const glm::vec3 offset = candidate - p_point;
            const float distSq = glm::dot(offset, offset);
            if (distSq < bestDistSq)
            {
                bestDistSq = distSq;
                best = candidate;
            }
        }
        return best;
    }

    /// Index of the box that contains the point most deeply, or -1 when no box contains it.
    int DeepestContainingBox(const std::vector<OBB> &p_boxes, const glm::vec3 &p_point, float &p_depth)
    {
        int deepest = -1;
        p_depth = 0.0f;
        for (std::size_t i = 0; i < p_boxes.size(); ++i)
        {
            const float depth = BoxDepth(p_boxes[i], p_point);
            if (depth >= -BOUNDS_TOLERANCE && (deepest < 0 || depth > p_depth))
            {
                deepest = static_cast<int>(i);
                p_depth = depth;
            }
        }
        return deepest;
    }

    /// Moves a position that has left every box onto the nearest box surface, and removes the velocity pointing out of the box.
    void ConstrainToBoxes(const std::vector<OBB> &p_boxes, glm::vec3 &p_position, glm::vec3 &p_velocity)
    {
        if (p_boxes.empty())
        {
            return;
        }

        float depth;
        if (DeepestContainingBox(p_boxes, p_position, depth) >= 0)
        {
            return;
        }

        const glm::vec3 surface = ClosestPointOnBoxes(p_boxes, p_position);
        const glm::vec3 outward = p_position - surface;
        const float length = glm::length(outward);
        if (length > EPSILON)
        {
            const glm::vec3 normal = outward / length;
            const float outwardSpeed = glm::dot(p_velocity, normal);
            if (outwardSpeed > 0.0f)
            {
                p_velocity -= normal * outwardSpeed;
            }
        }
        p_position = surface;
    }

    glm::vec3 SeekSteer(const glm::vec3 &p_target, const glm::vec3 &p_position, const glm::vec3 &p_velocity, float p_maxSpeed)
    {
        const glm::vec3 offset = p_target - p_position;
        const float distance = glm::length(offset);
        const glm::vec3 desired = distance > EPSILON ? offset / distance * p_maxSpeed : glm::vec3(0.0f);
        return desired - p_velocity;
    }

    /// Steers toward the nearest box when outside every box, or toward the deepest box's centre when too close to its faces.
    glm::vec3 ComputeBoundsSteer(const std::vector<OBB> &p_boxes, const glm::vec3 &p_position, const glm::vec3 &p_velocity, float p_margin, float p_maxSpeed)
    {
        if (p_boxes.empty())
        {
            return glm::vec3(0.0f);
        }

        float depth;
        const int deepest = DeepestContainingBox(p_boxes, p_position, depth);
        if (deepest < 0)
        {
            return SeekSteer(ClosestPointOnBoxes(p_boxes, p_position), p_position, p_velocity, p_maxSpeed);
        }
        if (depth >= p_margin)
        {
            return glm::vec3(0.0f);
        }
        return SeekSteer(p_boxes[deepest].center, p_position, p_velocity, p_maxSpeed);
    }

    /// The point p_lookahead further along the closed loop from the point on the loop nearest to p_position.
    glm::vec3 SamplePathTarget(const std::vector<glm::vec3> &p_points, const glm::vec3 &p_position, float p_lookahead)
    {
        ZoneScoped;

        const std::size_t count = p_points.size();

        std::size_t nearest = 0;
        float nearestT = 0.0f;
        float nearestDistSq = std::numeric_limits<float>::max();
        float totalLength = 0.0f;
        for (std::size_t i = 0; i < count; ++i)
        {
            const glm::vec3 &a = p_points[i];
            const glm::vec3 &b = p_points[(i + 1) % count];
            const glm::vec3 ab = b - a;
            const float abLengthSq = glm::dot(ab, ab);
            const float t = abLengthSq > EPSILON ? std::clamp(glm::dot(p_position - a, ab) / abLengthSq, 0.0f, 1.0f) : 0.0f;
            const glm::vec3 offset = a + ab * t - p_position;
            const float distSq = glm::dot(offset, offset);
            if (distSq < nearestDistSq)
            {
                nearestDistSq = distSq;
                nearest = i;
                nearestT = t;
            }
            totalLength += std::sqrt(abLengthSq);
        }

        if (totalLength < EPSILON)
        {
            return p_points[0];
        }

        std::size_t segment = nearest;
        float offsetOnSegment = nearestT * glm::length(p_points[(nearest + 1) % count] - p_points[nearest]);
        float remaining = std::fmod(std::max(p_lookahead, 0.0f), totalLength);
        for (std::size_t step = 0; step <= count; ++step)
        {
            const glm::vec3 &a = p_points[segment];
            const glm::vec3 &b = p_points[(segment + 1) % count];
            const float length = glm::length(b - a);
            if (offsetOnSegment + remaining <= length)
            {
                const float t = length > EPSILON ? (offsetOnSegment + remaining) / length : 0.0f;
                return a + (b - a) * t;
            }

            remaining -= length - offsetOnSegment;
            offsetOnSegment = 0.0f;
            segment = (segment + 1) % count;
        }
        return p_points[segment];
    }

    void WriteNodeIDs(nlohmann::json &p_json, const char *p_key, const std::vector<std::weak_ptr<Node>> &p_nodes,
        const std::vector<std::uint64_t> &p_unresolved)
    {
        nlohmann::json ids = nlohmann::json::array();
        for (const std::weak_ptr<Node> &weakNode : p_nodes)
        {
            if (const std::shared_ptr<Node> node = weakNode.lock())
            {
                ids.push_back(node->GetID());
            }
        }
        for (const std::uint64_t id : p_unresolved)
        {
            ids.push_back(id);
        }
        p_json[p_key] = ids;
    }

    void ReadNodeIDs(const nlohmann::json &p_json, const char *p_key, std::vector<std::uint64_t> &p_unresolved)
    {
        p_unresolved.clear();
        if (p_json.contains(p_key))
        {
            for (const nlohmann::json &id : p_json.at(p_key))
            {
                p_unresolved.push_back(id.get<std::uint64_t>());
            }
        }
    }

    void ResolveNodeIDs(SerializationContext &p_context, std::vector<std::uint64_t> &p_unresolved, std::vector<std::weak_ptr<Node>> &p_nodes)
    {
        std::vector<std::uint64_t> stillUnresolved;
        for (const std::uint64_t id : p_unresolved)
        {
            const std::shared_ptr<Node> node = p_context.FindNode(id);
            if (!node)
            {
                stillUnresolved.push_back(id);
            }
            else if (!ContainsNode(p_nodes, node.get()))
            {
                p_nodes.push_back(node);
            }
        }
        p_unresolved = std::move(stillUnresolved);
    }
}

namespace Droplet::Scene
{
    void BoidsComponent::AddFish(const std::shared_ptr<Node> &p_fish)
    {
        if (!p_fish || ContainsNode(m_fish, p_fish.get()))
        {
            return;
        }

        m_fish.push_back(p_fish);
    }

    void BoidsComponent::RemoveFish(const std::shared_ptr<Node> &p_fish)
    {
        if (!p_fish)
        {
            return;
        }

        std::erase_if(m_fish, [&p_fish](const std::weak_ptr<Node> &p_weak)
        {
            return p_weak.lock() == p_fish;
        });
    }

    std::vector<std::shared_ptr<Node>> BoidsComponent::GetFish() const
    {
        std::vector<std::shared_ptr<Node>> fish;
        fish.reserve(m_fish.size());
        for (const std::weak_ptr<Node> &weakFish : m_fish)
        {
            if (std::shared_ptr<Node> node = weakFish.lock())
            {
                fish.push_back(node);
            }
        }
        return fish;
    }

    struct BoidsComponent::Flock
    {
        std::vector<std::shared_ptr<Node>> nodes;
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> velocities;
        std::vector<glm::vec3> headings;
        std::vector<std::size_t> live;
        std::vector<std::weak_ptr<Node>> ignoreNodes;
        std::vector<OBB> boxes;
        std::vector<glm::vec3> pathPoints;
    };

    struct BoidsComponent::SteeringForces
    {
        /// Weighted terms. The total is their sum, clamped to the max steering force.
        glm::vec3 separation{0.0f};
        glm::vec3 alignment{0.0f};
        glm::vec3 cohesion{0.0f};
        glm::vec3 avoidance{0.0f};
        glm::vec3 bounds{0.0f};
        glm::vec3 path{0.0f};
        glm::vec3 total{0.0f};

        glm::vec3 neighbourCentre{0.0f};
        std::size_t neighbourCount = 0;

        glm::vec3 pathTarget{0.0f};
        bool hasPathTarget = false;
    };

    BoidsComponent::Flock BoidsComponent::GatherFlock()
    {
        const std::size_t boidCount = m_fish.size();
        Flock flock;
        flock.nodes.resize(boidCount);
        flock.positions.resize(boidCount);
        flock.velocities.resize(boidCount);
        flock.headings.resize(boidCount);
        flock.live.reserve(boidCount);

        flock.ignoreNodes.push_back(GetOwner());

        if (m_mode == BoidsMode::Bounds)
        {
            ZoneNamed(computeBoundsZone, "Compute Bounds");

            for (const std::weak_ptr<Node> &weakBound : m_bounds)
            {
                if (std::shared_ptr<Node> bound = weakBound.lock())
                {
                    flock.boxes.push_back(GetNodeBox(*bound));
                    flock.ignoreNodes.push_back(bound);
                    for (const std::shared_ptr<Node> &child : bound->GetChildren(true))
                    {
                        flock.ignoreNodes.push_back(child);
                    }
                }
            }
        }

        if (m_mode == BoidsMode::Path)
        {
            ZoneNamed(computePathZone, "Compute Path");

            for (const std::weak_ptr<Node> &weakPoint : m_path)
            {
                if (const std::shared_ptr<Node> point = weakPoint.lock())
                {
                    flock.pathPoints.push_back(point->GetTransform().GetPosition(Transform::Space::World));
                    flock.ignoreNodes.push_back(point);
                    for (const std::shared_ptr<Node> &child : point->GetChildren(true))
                    {
                        flock.ignoreNodes.push_back(child);
                    }
                }
            }
        }

        for (std::size_t i = 0; i < boidCount; ++i)
        {
            ZoneNamed(fetchBoidZone, "Fetch Boid");

            flock.nodes[i] = m_fish[i].lock();
            if (!flock.nodes[i])
            {
                continue;
            }

            flock.live.push_back(i);
            const Transform &transform = flock.nodes[i]->GetTransform();
            flock.positions[i] = transform.GetPosition(Transform::Space::World);

            const auto velocityIt = m_velocities.find(flock.nodes[i].get());
            glm::vec3 velocity = velocityIt != m_velocities.end() ? velocityIt->second : glm::vec3(0.0f);
            if (glm::dot(velocity, velocity) < EPSILON)
            {
                velocity = transform.GetForward(Transform::Space::World) * m_minSpeed;
            }
            flock.velocities[i] = velocity;

            const float speed = glm::length(velocity);
            flock.headings[i] = speed > EPSILON ? velocity / speed : transform.GetForward(Transform::Space::World);

            flock.ignoreNodes.push_back(flock.nodes[i]);
            for (const std::shared_ptr<Node> &child : flock.nodes[i]->GetChildren(true))
            {
                flock.ignoreNodes.push_back(child);
            }
        }

        return flock;
    }

    void BoidsComponent::ComputeSteering(Scene &p_scene, Flock &p_flock, std::vector<SteeringForces> &p_forces)
    {
        p_forces.assign(p_flock.nodes.size(), SteeringForces{});

        const float perceptionRadiusSq = m_perceptionRadius * m_perceptionRadius;
        const float separationRadiusSq = m_separationRadius * m_separationRadius;

        for (std::size_t i : p_flock.live)
        {
            ZoneNamed(computeSteeringZone, "Compute Steering");

            const glm::vec3 position = p_flock.positions[i];
            const glm::vec3 velocity = p_flock.velocities[i];
            const glm::vec3 heading = p_flock.headings[i];

            glm::vec3 separation(0.0f);
            glm::vec3 averageVelocity(0.0f);
            glm::vec3 centre(0.0f);
            std::size_t neighbourCount = 0;

            for (std::size_t j : p_flock.live)
            {
                if (j == i)
                {
                    continue;
                }

                const glm::vec3 offset = position - p_flock.positions[j];
                const float distanceSq = glm::dot(offset, offset);
                if (distanceSq > perceptionRadiusSq)
                {
                    continue;
                }

                ++neighbourCount;
                averageVelocity += p_flock.velocities[j];
                centre += p_flock.positions[j];

                if (distanceSq < separationRadiusSq && distanceSq > EPSILON)
                {
                    separation += offset / distanceSq;
                }
            }

            SteeringForces &forces = p_forces[i];
            forces.separation = separation * m_separationWeight;
            forces.neighbourCount = neighbourCount;
            if (neighbourCount > 0)
            {
                const float inverseCount = 1.0f / static_cast<float>(neighbourCount);
                forces.neighbourCentre = centre * inverseCount;
                forces.alignment = (averageVelocity * inverseCount - velocity) * m_alignmentWeight;
                forces.cohesion = (forces.neighbourCentre - position) * m_cohesionWeight;
            }

            if (m_avoidanceDistance > EPSILON)
            {
                forces.avoidance = ComputeAvoidance(p_scene, position, heading, p_flock.ignoreNodes, m_avoidanceDistance, m_avoidanceLayerMask) * m_avoidanceWeight;
            }

            if (m_mode == BoidsMode::Bounds)
            {
                forces.bounds = ComputeBoundsSteer(p_flock.boxes, position, velocity, m_boundsMargin, m_maxSpeed) * m_boundsWeight;
            }
            else if (m_mode == BoidsMode::Path && !p_flock.pathPoints.empty())
            {
                forces.pathTarget = SamplePathTarget(p_flock.pathPoints, position, m_pathLookahead);
                forces.hasPathTarget = true;
                forces.path = SeekSteer(forces.pathTarget, position, velocity, m_maxSpeed) * m_pathWeight;
            }

            glm::vec3 steering = forces.separation + forces.alignment + forces.cohesion + forces.avoidance + forces.bounds + forces.path;
            const float steeringLength = glm::length(steering);
            if (steeringLength > m_maxSteeringForce && steeringLength > EPSILON)
            {
                steering *= m_maxSteeringForce / steeringLength;
            }
            forces.total = steering;
        }
    }

    void BoidsComponent::UpdateImpl()
    {
        const float deltaTime = std::min(Droplet::Time::Get().GetDeltaTime(), MAX_DELTA_TIME);
        if (deltaTime <= 0.0f)
        {
            return;
        }

        const std::shared_ptr<Scene> scene = TryGetScene();
        if (!scene)
        {
            return;
        }

        Flock flock = GatherFlock();
        std::vector<SteeringForces> forces;
        ComputeSteering(*scene, flock, forces);

        m_velocities.clear();
        for (std::size_t i : flock.live)
        {
            ZoneNamed(applySteeringZone, "Apply Steering");

            const glm::vec3 newVelocity = flock.velocities[i] + forces[i].total * deltaTime;
            const float newSpeed = glm::length(newVelocity);
            const glm::vec3 newHeading = newSpeed > EPSILON ? newVelocity / newSpeed : flock.headings[i];
            glm::vec3 velocity = newHeading * std::min(std::max(newSpeed, m_minSpeed), m_maxSpeed);

            glm::vec3 newPosition = flock.positions[i] + velocity * deltaTime;
            if (m_mode == BoidsMode::Bounds)
            {
                ConstrainToBoxes(flock.boxes, newPosition, velocity);
            }
            m_velocities[flock.nodes[i].get()] = velocity;

            Transform &transform = flock.nodes[i]->GetTransform();
            transform.SetPosition(newPosition, Transform::Space::World);

            const float speed = glm::length(velocity);
            if (speed > EPSILON)
            {
                const glm::vec3 direction = velocity / speed;
                const glm::vec3 up = std::abs(direction.y) < 0.99f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(0.0f, 0.0f, 1.0f);
                transform.LookAt(newPosition + direction, up, Transform::Space::World);
            }
        }
    }

    void BoidsComponent::EditorUpdateImpl()
    {
        const bool selected = m_uiSelected;
        m_uiSelected = false;

        if (m_debugInfo)
        {
            DrawBoundsAndPath();
            DrawSteeringDebug();
        }
        else if (selected)
        {
            DrawBoundsAndPath();
        }
    }

    void BoidsComponent::DrawBoundsAndPath()
    {
        using Droplet::Graphics::Debug::DebugDraw;

        const glm::vec4 boundsColour(1.0f, 0.6f, 0.0f, 1.0f);
        for (const std::weak_ptr<Node> &weakBound : m_bounds)
        {
            if (const std::shared_ptr<Node> bound = weakBound.lock())
            {
                DebugDraw::DrawBox(GetNodeBox(*bound), boundsColour, true, true);
            }
        }

        const glm::vec4 pathColour(0.0f, 0.8f, 1.0f, 1.0f);
        std::vector<glm::vec3> points;
        for (const std::weak_ptr<Node> &weakPoint : m_path)
        {
            if (const std::shared_ptr<Node> point = weakPoint.lock())
            {
                points.push_back(point->GetTransform().GetPosition(Transform::Space::World));
            }
        }

        for (std::size_t i = 0; i < points.size(); ++i)
        {
            DebugDraw::DrawLine(points[i], points[(i + 1) % points.size()], pathColour);
            DebugDraw::DrawWireSphere(Sphere(points[i], DEBUG_MARKER_RADIUS), pathColour, 8);
        }
    }

    void BoidsComponent::DrawSteeringDebug()
    {
        using Droplet::Graphics::Debug::DebugDraw;

        const std::shared_ptr<Scene> scene = TryGetScene();
        if (!scene)
        {
            return;
        }

        Flock flock = GatherFlock();
        std::vector<SteeringForces> forces;
        ComputeSteering(*scene, flock, forces);

        const glm::vec4 perceptionColour(0.5f, 0.5f, 0.5f, 0.5f);
        const glm::vec4 clusterColour(1.0f, 1.0f, 1.0f, 0.6f);
        const glm::vec4 separationColour(1.0f, 0.2f, 0.2f, 1.0f);
        const glm::vec4 alignmentColour(0.2f, 1.0f, 0.2f, 1.0f);
        const glm::vec4 cohesionColour(0.2f, 0.4f, 1.0f, 1.0f);
        const glm::vec4 avoidanceColour(1.0f, 1.0f, 0.2f, 1.0f);
        const glm::vec4 boundsColour(1.0f, 0.2f, 1.0f, 1.0f);
        const glm::vec4 pathColour(0.2f, 1.0f, 1.0f, 1.0f);
        const glm::vec4 totalColour(1.0f, 1.0f, 1.0f, 1.0f);

        glm::vec3 flockCentre(0.0f);
        for (std::size_t i : flock.live)
        {
            const glm::vec3 position = flock.positions[i];
            const SteeringForces &steer = forces[i];
            flockCentre += position;

            DebugDraw::DrawWireSphere(Sphere(position, m_perceptionRadius), perceptionColour, 12);
            if (steer.neighbourCount > 0)
            {
                DebugDraw::DrawLine(position, steer.neighbourCentre, clusterColour);
            }
            if (steer.hasPathTarget)
            {
                DebugDraw::DrawLine(position, steer.pathTarget, pathColour);
            }

            const auto drawForce = [&position](const glm::vec3 &p_force, const glm::vec4 &p_colour)
            {
                DebugDraw::DrawLine(position, position + p_force * DEBUG_FORCE_SCALE, p_colour);
            };
            drawForce(steer.separation, separationColour);
            drawForce(steer.alignment, alignmentColour);
            drawForce(steer.cohesion, cohesionColour);
            drawForce(steer.avoidance, avoidanceColour);
            drawForce(steer.bounds, boundsColour);
            drawForce(steer.path, pathColour);
            drawForce(steer.total, totalColour);
        }

        if (!flock.live.empty())
        {
            flockCentre /= static_cast<float>(flock.live.size());
            DebugDraw::DrawWireSphere(Sphere(flockCentre, DEBUG_MARKER_RADIUS), clusterColour, 8);
        }
    }

    void BoidsComponent::ResolveReferences(SerializationContext &p_context)
    {
        ResolveNodeIDs(p_context, m_unresolvedFishIDs, m_fish);
        ResolveNodeIDs(p_context, m_unresolvedBoundIDs, m_bounds);
        ResolveNodeIDs(p_context, m_unresolvedPathIDs, m_path);
    }

    void BoidsComponent::RenderUIImpl()
    {
#ifdef _EDITOR
        ImGui::PushID(this);

        m_uiSelected = true;

        int mode = static_cast<int>(m_mode);
        const char *modes[] = {"Free", "Bounds", "Path"};
        if (ImGui::Combo("Mode", &mode, modes, IM_COUNTOF(modes)))
        {
            m_mode = static_cast<BoidsMode>(mode);
        }
        ImGui::Checkbox("Debug Info", &m_debugInfo);

        const auto acceptFish = [this](Node *p_node)
        {
            return p_node != nullptr && !ContainsNode(m_fish, p_node);
        };
        Droplet::Editor::NodeListSlot("Fish", m_fish, -1, acceptFish, std::nullopt, "This node is already in the flock");

        if (m_mode == BoidsMode::Bounds)
        {
            const auto acceptBound = [this](Node *p_node)
            {
                return p_node != nullptr && !ContainsNode(m_bounds, p_node);
            };
            Droplet::Editor::NodeListSlot("Bounds", m_bounds, -1, acceptBound, std::nullopt, "This node is already a bounds volume");
            ImGui::DragFloat("Bounds Weight", &m_boundsWeight, 0.05f, 0.0f, 0.0f, "%.2f");
            ImGui::DragFloat("Bounds Margin", &m_boundsMargin, 0.05f, 0.0f, 0.0f, "%.2f");
        }
        else if (m_mode == BoidsMode::Path)
        {
            const auto acceptPoint = [this](Node *p_node)
            {
                return p_node != nullptr && !ContainsNode(m_path, p_node);
            };
            Droplet::Editor::NodeListSlot("Path Points", m_path, -1, acceptPoint, std::nullopt, "This node is already a path point");
            ImGui::DragFloat("Path Weight", &m_pathWeight, 0.05f, 0.0f, 0.0f, "%.2f");
            ImGui::DragFloat("Path Lookahead", &m_pathLookahead, 0.05f, 0.0f, 0.0f, "%.2f");
        }

        ImGui::DragFloat("Perception Radius", &m_perceptionRadius, 0.05f, 0.0f, 0.0f, "%.2f");
        ImGui::DragFloat("Separation Radius", &m_separationRadius, 0.05f, 0.0f, 0.0f, "%.2f");
        ImGui::DragFloat("Separation Weight", &m_separationWeight, 0.05f, 0.0f, 0.0f, "%.2f");
        ImGui::DragFloat("Alignment Weight", &m_alignmentWeight, 0.05f, 0.0f, 0.0f, "%.2f");
        ImGui::DragFloat("Cohesion Weight", &m_cohesionWeight, 0.05f, 0.0f, 0.0f, "%.2f");
        ImGui::DragFloat("Min Speed", &m_minSpeed, 0.05f, 0.0f, 0.0f, "%.2f");
        ImGui::DragFloat("Max Speed", &m_maxSpeed, 0.05f, 0.0f, 0.0f, "%.2f");
        ImGui::DragFloat("Max Steering Force", &m_maxSteeringForce, 0.05f, 0.0f, 0.0f, "%.2f");
        ImGui::DragFloat("Avoidance Distance", &m_avoidanceDistance, 0.05f, 0.0f, 0.0f, "%.2f");
        ImGui::DragFloat("Avoidance Weight", &m_avoidanceWeight, 0.05f, 0.0f, 0.0f, "%.2f");

		CollisionLayerUtility::SetLayerMaskUI("Avoidance Layer Mask", m_avoidanceLayerMask);

        ImGui::PopID();
#endif
    }

    nlohmann::json BoidsComponent::SerializeImpl()
    {
        nlohmann::json json;
        WriteNodeIDs(json, "fish", m_fish, m_unresolvedFishIDs);
        WriteNodeIDs(json, "bounds", m_bounds, m_unresolvedBoundIDs);
        WriteNodeIDs(json, "path", m_path, m_unresolvedPathIDs);
        json["avoidanceLayerMask"] = static_cast<std::underlying_type_t<Droplet::CollisionLayer>>(m_avoidanceLayerMask);

        if (m_mode == BoidsMode::Bounds)
        {
            json["mode"] = "bounds";
        }
        else if (m_mode == BoidsMode::Path)
        {
            json["mode"] = "path";
        }
        else
        {
            json["mode"] = "free";
        }

        json["perceptionRadius"] = m_perceptionRadius;
        json["separationRadius"] = m_separationRadius;
        json["separationWeight"] = m_separationWeight;
        json["alignmentWeight"] = m_alignmentWeight;
        json["cohesionWeight"] = m_cohesionWeight;
        json["minSpeed"] = m_minSpeed;
        json["maxSpeed"] = m_maxSpeed;
        json["maxSteeringForce"] = m_maxSteeringForce;
        json["avoidanceDistance"] = m_avoidanceDistance;
        json["avoidanceWeight"] = m_avoidanceWeight;
        json["boundsWeight"] = m_boundsWeight;
        json["boundsMargin"] = m_boundsMargin;
        json["pathWeight"] = m_pathWeight;
        json["pathLookahead"] = m_pathLookahead;
        return json;
    }

    void BoidsComponent::DeserializeImpl(const nlohmann::json p_compJson)
    {
        if (p_compJson.empty())
        {
            return;
        }

        m_fish.clear();
        m_bounds.clear();
        m_path.clear();
        m_velocities.clear();
        ReadNodeIDs(p_compJson, "fish", m_unresolvedFishIDs);
        ReadNodeIDs(p_compJson, "bounds", m_unresolvedBoundIDs);
        ReadNodeIDs(p_compJson, "path", m_unresolvedPathIDs);

        if (p_compJson.contains("avoidanceLayerMask"))
        {
            m_avoidanceLayerMask = static_cast<Droplet::CollisionLayer>(p_compJson.at("avoidanceLayerMask").get<std::uint16_t>());
		}

        if (p_compJson.contains("mode"))
        {
            const std::string mode = p_compJson.at("mode").get<std::string>();
            if (mode == "bounds")
            {
                m_mode = BoidsMode::Bounds;
            }
            else if (mode == "path")
            {
                m_mode = BoidsMode::Path;
            }
            else
            {
                m_mode = BoidsMode::Free;
            }
        }

        ReadValue(p_compJson, "perceptionRadius", m_perceptionRadius);
        ReadValue(p_compJson, "separationRadius", m_separationRadius);
        ReadValue(p_compJson, "separationWeight", m_separationWeight);
        ReadValue(p_compJson, "alignmentWeight", m_alignmentWeight);
        ReadValue(p_compJson, "cohesionWeight", m_cohesionWeight);
        ReadValue(p_compJson, "minSpeed", m_minSpeed);
        ReadValue(p_compJson, "maxSpeed", m_maxSpeed);
        ReadValue(p_compJson, "maxSteeringForce", m_maxSteeringForce);
        ReadValue(p_compJson, "avoidanceDistance", m_avoidanceDistance);
        ReadValue(p_compJson, "avoidanceWeight", m_avoidanceWeight);
        ReadValue(p_compJson, "boundsWeight", m_boundsWeight);
        ReadValue(p_compJson, "boundsMargin", m_boundsMargin);
        ReadValue(p_compJson, "pathWeight", m_pathWeight);
        ReadValue(p_compJson, "pathLookahead", m_pathLookahead);
    }
}
