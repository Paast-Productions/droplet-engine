#pragma once

#include <utility>
#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

class Component;

class Node : public std::enable_shared_from_this<Node>
{
public:
    explicit Node(std::string p_name);
    virtual ~Node() = default;

    virtual void Update(float p_deltaTime);
    virtual void Render();

    // Children

    std::shared_ptr<Node> AddChild(std::shared_ptr<Node> p_child);
    void RemoveChild(std::shared_ptr<Node> p_child);

    std::shared_ptr<Node> GetParent() const;

    const std::vector<std::shared_ptr<Node>>& GetChildren() const;

    // Name

    const std::string& GetName() const;

    // Components

    template<typename T, typename... Args>
    std::shared_ptr<T> AddComponent(Args&&... p_args)
    {
        auto component =
            std::make_shared<T>(
                std::forward<Args>(p_args)...);

        component->SetOwner(shared_from_this());

        m_components.push_back(component);

        component->Initialize();

        return component;
    }

    template<typename T>
    std::shared_ptr<T> GetComponent() const
    {
        for (const auto& component : m_components)
        {
            auto result =
                std::dynamic_pointer_cast<T>(component);

            if (result)
            {
                return result;
            }
        }

        return nullptr;
    }

    template<typename T>
    bool RemoveComponent()
    {
        auto it = std::find_if(
            m_components.begin(),
            m_components.end(),
            [](const std::shared_ptr<Component>& p_component)
            {
                return std::dynamic_pointer_cast<T>(p_component)
                    != nullptr;
            });

        if (it == m_components.end())
        {
            return false;
        }

        m_components.erase(it);

        return true;
    }

    // Transforms

    void SetPosition(const glm::vec3& p_position);
    void SetRotation(const glm::vec3& p_rotation);
    void SetScale(const glm::vec3& p_scale);

    const glm::vec3& GetPosition() const;
    const glm::vec3& GetRotation() const;
    const glm::vec3& GetScale() const;

    const glm::mat4& GetLocalTransform() const;
    const glm::mat4& GetWorldTransform() const;

private:
    std::string m_name;

    std::weak_ptr<Node> m_parent;
    std::vector<std::shared_ptr<Node>> m_children;

	// Components

    std::vector<std::shared_ptr<Component>> m_components;

    // Transforms

    glm::vec3 m_position{ 0.0f, 0.0f, 0.0f };
    glm::vec3 m_rotation{ 0.0f, 0.0f, 0.0f };
    glm::vec3 m_scale{ 1.0f, 1.0f, 1.0f };

    glm::mat4 m_localTransform{ 1.0f };
    glm::mat4 m_worldTransform{ 1.0f };

    bool m_active = true;

    
};