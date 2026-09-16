#pragma once

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>

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

private:
    std::string m_name;

    std::weak_ptr<Node> m_parent;
    std::vector<std::shared_ptr<Node>> m_children;
    
    //transforms

    bool m_active = true;
};