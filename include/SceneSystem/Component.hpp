#pragma once

#include <memory>

class Node;

class Component
{
public:
    Component() = default;
    virtual ~Component() = default;

    virtual void Initialize() {}
    virtual void Update([[maybe_unused]] float p_deltaTime) {}

    std::shared_ptr<Node> GetOwner() const;

private:
    friend class Node;

    void SetOwner(std::shared_ptr<Node> p_owner);

    std::weak_ptr<Node> m_owner;
};