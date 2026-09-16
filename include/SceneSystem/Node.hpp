#pragma once

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

class Node : public std::enable_shared_from_this<Node>
{
public:
    explicit Node(const std::string& p_name);
    virtual ~Node() = default;

    virtual void Update(float p_deltaTime);
    virtual void Render();

    std::shared_ptr<Node> AddChild(std::shared_ptr<Node> p_child);
    void RemoveChild(std::shared_ptr<Node> p_child);

    std::shared_ptr<Node> GetParent() const;

    const std::vector<std::shared_ptr<Node>>& GetChildren() const;

    const std::string& GetName() const;

    //tranforms

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
    
    //transforms
    glm::vec3 m_position{ 0.0f, 0.0f, 0.0f };
    glm::vec3 m_rotation{ 0.0f, 0.0f, 0.0f };
    glm::vec3 m_scale{ 1.0f, 1.0f, 1.0f };

    glm::mat4 m_localTransform{ 1.0f };
    glm::mat4 m_worldTransform{ 1.0f };

    bool m_active = true;
};