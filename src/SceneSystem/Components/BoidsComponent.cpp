#include "BoidsComponent.hpp"

#include <algorithm>
#include <cmath>

#include <ImGui/imgui.h>

#ifdef _EDITOR
#include <Editor/NodeSlot.hpp>
#endif

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
        std::vector<std::weak_ptr<Node>> &p_ignoreNodes, float p_range)
    {
        RayHit forwardHit;
        if (p_scene.RaycastNode(Ray(p_position, p_heading), &forwardHit, &p_ignoreNodes, Droplet::CollisionLayer::All, p_range).expired())
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
            const bool blocked = !p_scene.RaycastNode(Ray(p_position, direction), &sideHit, &p_ignoreNodes, Droplet::CollisionLayer::All, p_range).expired();
            const float freeDistance = blocked ? sideHit.distance : p_range;
            if (freeDistance > bestDistance)
            {
                bestDistance = freeDistance;
                bestDirection = direction;
            }
        }

        return (bestDirection - p_heading) * urgency;
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

        const std::size_t boidCount = m_fish.size();
        std::vector<std::shared_ptr<Node>> nodes(boidCount);
        std::vector<glm::vec3> velocities(boidCount);
        std::vector<glm::vec3> positions(boidCount);
        std::vector<std::size_t> live;
        live.reserve(boidCount);

        std::vector<std::weak_ptr<Node>> ignoreNodes;
        ignoreNodes.push_back(GetOwner());

        for (std::size_t i = 0; i < boidCount; ++i)
        {
            nodes[i] = m_fish[i].lock();
            if (!nodes[i])
            {
                continue;
            }

            live.push_back(i);
            const Transform &transform = nodes[i]->GetTransform();
            positions[i] = transform.GetPosition(Transform::Space::World);

            const auto velocityIt = m_velocities.find(nodes[i].get());
            glm::vec3 velocity = velocityIt != m_velocities.end() ? velocityIt->second : glm::vec3(0.0f);
            if (glm::dot(velocity, velocity) < EPSILON)
            {
                velocity = transform.GetForward(Transform::Space::World) * m_minSpeed;
            }
            velocities[i] = velocity;

            ignoreNodes.push_back(nodes[i]);
            for (const std::shared_ptr<Node> &child : nodes[i]->GetChildren(true))
            {
                ignoreNodes.push_back(child);
            }
        }

        const float perceptionRadiusSq = m_perceptionRadius * m_perceptionRadius;
        const float separationRadiusSq = m_separationRadius * m_separationRadius;
        std::vector<glm::vec3> newVelocities(boidCount);

        for (std::size_t i : live)
        {
            const glm::vec3 position = positions[i];
            const glm::vec3 velocity = velocities[i];
            const float speed = glm::length(velocity);
            const glm::vec3 heading = speed > EPSILON ? velocity / speed : nodes[i]->GetTransform().GetForward(Transform::Space::World);

            glm::vec3 separation(0.0f);
            glm::vec3 averageVelocity(0.0f);
            glm::vec3 centre(0.0f);
            std::size_t neighbourCount = 0;

            for (std::size_t j : live)
            {
                if (j == i)
                {
                    continue;
                }

                const glm::vec3 offset = position - positions[j];
                const float distanceSq = glm::dot(offset, offset);
                if (distanceSq > perceptionRadiusSq)
                {
                    continue;
                }

                ++neighbourCount;
                averageVelocity += velocities[j];
                centre += positions[j];

                if (distanceSq < separationRadiusSq && distanceSq > EPSILON)
                {
                    separation += offset / distanceSq;
                }
            }

            glm::vec3 steering = separation * m_separationWeight;
            if (neighbourCount > 0)
            {
                const float inverseCount = 1.0f / static_cast<float>(neighbourCount);
                steering += (averageVelocity * inverseCount - velocity) * m_alignmentWeight;
                steering += (centre * inverseCount - position) * m_cohesionWeight;
            }

            if (m_avoidanceDistance > EPSILON)
            {
                steering += ComputeAvoidance(*scene, position, heading, ignoreNodes, m_avoidanceDistance) * m_avoidanceWeight;
            }

            const float steeringLength = glm::length(steering);
            if (steeringLength > m_maxSteeringForce && steeringLength > EPSILON)
            {
                steering *= m_maxSteeringForce / steeringLength;
            }

            const glm::vec3 newVelocity = velocity + steering * deltaTime;
            const float newSpeed = glm::length(newVelocity);
            const glm::vec3 newHeading = newSpeed > EPSILON ? newVelocity / newSpeed : heading;
            newVelocities[i] = newHeading * std::min(std::max(newSpeed, m_minSpeed), m_maxSpeed);
        }

        m_velocities.clear();
        for (std::size_t i : live)
        {
            const glm::vec3 &velocity = newVelocities[i];
            m_velocities[nodes[i].get()] = velocity;

            const glm::vec3 newPosition = positions[i] + velocity * deltaTime;
            Transform &transform = nodes[i]->GetTransform();
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

    void BoidsComponent::ResolveReferences(SerializationContext &p_context)
    {
        std::vector<std::uint64_t> unresolved;
        for (const std::uint64_t fishID : m_unresolvedFishIDs)
        {
            std::shared_ptr<Node> fish = p_context.FindNode(fishID);
            if (!fish)
            {
                unresolved.push_back(fishID);
            }
            else if (!ContainsNode(m_fish, fish.get()))
            {
                m_fish.push_back(fish);
            }
        }
        m_unresolvedFishIDs = std::move(unresolved);
    }

    void BoidsComponent::RenderUIImpl()
    {
#ifdef _EDITOR
        ImGui::PushID(this);

        const auto acceptFish = [this](Node *p_node)
        {
            return p_node != nullptr && !ContainsNode(m_fish, p_node);
        };
        Droplet::Editor::NodeListSlot("Fish", m_fish, -1, acceptFish, std::nullopt, "This node is already in the flock");

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

        ImGui::PopID();
#endif
    }

    nlohmann::json BoidsComponent::SerializeImpl()
    {
        nlohmann::json fishIDs = nlohmann::json::array();
        for (const std::weak_ptr<Node> &weakFish : m_fish)
        {
            if (const std::shared_ptr<Node> fish = weakFish.lock())
            {
                fishIDs.push_back(fish->GetID());
            }
        }
        for (const std::uint64_t fishID : m_unresolvedFishIDs)
        {
            fishIDs.push_back(fishID);
        }

        nlohmann::json json;
        json["fish"] = fishIDs;
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
        return json;
    }

    void BoidsComponent::DeserializeImpl(const nlohmann::json p_compJson)
    {
        if (p_compJson.empty())
        {
            return;
        }

        m_fish.clear();
        m_unresolvedFishIDs.clear();
        m_velocities.clear();
        if (p_compJson.contains("fish"))
        {
            for (const nlohmann::json &fishID : p_compJson.at("fish"))
            {
                m_unresolvedFishIDs.push_back(fishID.get<std::uint64_t>());
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
    }
}
