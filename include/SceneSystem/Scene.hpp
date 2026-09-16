#pragma once

#include <memory>
#include <string>
#include "Node.hpp"

class Node;

class Scene
{
public:
    explicit Scene(const std::string& p_name)
        : m_name(p_name), m_root(std::make_shared<Node>("Root"))
    {
    }
    virtual ~Scene() = default;

    virtual void Load();
    virtual void Unload();

    virtual void Update(float p_deltaTime);
    virtual void Render();

	std::shared_ptr<Node> GetRoot() const;

    const std::string& GetName() const;

    bool IsLoaded() const;
    bool IsActive() const;

    void SetActive(bool active);

private:
    std::string m_name;
    std::shared_ptr<Node> m_root;

    bool m_loaded = false;
    bool m_active = false;

};