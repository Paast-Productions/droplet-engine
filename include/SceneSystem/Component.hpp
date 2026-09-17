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
    /**
      * @brief Constructs a Node with the specified name.
      *
      * @param p_name Name of the Node.
      */
    Component() = default;

    /**
    * @brief Virtual destructor.
    */
    virtual ~Component() = default;

    /**
     * @brief Updates the Node and its Components and children.
     *
     * Updates the Node's transforms, followed by its Components
     * and child Nodes.
     *
     * @param p_deltaTime Time elapsed since the previous update, in seconds.
     */
    virtual void Initialize() {}

    /**
     * @brief Updates the component.
     *
     * Called once per update for components attached to active Nodes.
     *
     * @param p_deltaTime Time elapsed since the previous update, in seconds.
     */
    virtual void Update([[maybe_unused]] float p_deltaTime) {}
     
    /**
     * @brief Gets the Node that owns this component.
     *
     * @return A shared pointer to the owning Node, or nullptr if the
     *         owner no longer exists.
     */
    std::shared_ptr<Node> GetOwner() const;

private:
    friend class Node;

    /**
    * @brief Sets the Node that owns this component.
    *
    * Only Node can set the owner of a Component. This is used when
    * the component is added to a Node.
    *
    * @param p_owner Node that will own the component.
    */
    void SetOwner(std::shared_ptr<Node> p_owner);

    std::weak_ptr<Node> m_owner;
};