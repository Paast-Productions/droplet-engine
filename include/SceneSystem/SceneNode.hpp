#pragma once

#include <string>
#include <vector>
#include <memory>

class SceneNode : public std::enable_shared_from_this<SceneNode>
{
public:
    explicit SceneNode(const std::string& p_name);
    ~SceneNode() = default;

    std::shared_ptr<SceneNode> AddChild(std::shared_ptr<SceneNode> p_child);
    void RemoveChild(std::shared_ptr<SceneNode> p_child);

    std::shared_ptr<SceneNode> GetParent() const;

    const std::vector<std::shared_ptr<SceneNode>>& GetChildren() const; 

    const std::string& GetName() const;

    //tranforms

private:
    std::string m_name;

    std::weak_ptr<SceneNode> m_parent;
    std::vector<std::shared_ptr<SceneNode>> m_children;
    
    //transforms

    bool m_active = true;
};