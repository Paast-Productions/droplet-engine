#pragma once

#include <Component.hpp>
#include <Math/Bounds/AABB.hpp>
#include <Math/Bounds/OBB.hpp>
#include <Math/Bounds/Sphere.hpp>

namespace Droplet::Scene
{
    class TriggerComponent : public Component
    {
    public:
        TriggerComponent() = default;
        
        std::string_view GetTypeName() const override { return "TriggerComponent"; }
        
    protected:
        /// @brief Starts the component.
        void StartImpl() override;
		
        /// @brief Checks every fixed frame when to call trigger functions.
        void FixedUpdateImpl() override;
        
        /// @brief Update function that is mainly used for drawcalls
        void UpdateImpl() override;
		
        /// @brief Internal rendering function for the component's UI.
        ///
        /// Overloaded to provide a default UI for the ScriptComponent that
        /// displays the script path. Derived components can override this
        /// function to implement their own UI rendering logic.
        void RenderUIImpl() override;
    
    private:
        struct Trigger
        {
            glm::vec3 offset; // Offset from node position
        };
        struct TriggerAABB : Trigger
        {
            Math::AABB aabb;
        };
        struct TriggerOBB : Trigger
        {
            Math::OBB obb;
        };
        struct TriggerSphere : Trigger
        {
            Math::Sphere sphere;
        };
        
        bool m_isColliding = false;
        std::vector<std::weak_ptr<Node>> m_collidingNodes;
        
        // Trigger shapes
        std::vector<TriggerAABB> m_AABBs;
        std::vector<TriggerOBB> m_OBBs;
        std::vector<TriggerSphere> m_spheres;
    };
} 
