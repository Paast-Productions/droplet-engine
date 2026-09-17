#pragma once

#include <memory>

class Node;

/**
 * @brief Base class for components that can be attached to a Node.
 *
 * Components provide reusable functionality that can be added to Nodes
 * at runtime. Derived components can override Initialize() and Update()
 * to implement their own behavior.
 *
 * A Component is owned by the Node it is attached to and maintains a
 * weak reference to its owner to avoid an ownership cycle.
 */

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