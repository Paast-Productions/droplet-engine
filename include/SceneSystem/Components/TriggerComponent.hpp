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
        bool m_isColliding = false;
        std::vector<std::weak_ptr<Node>> m_collidingNodes;
        
        // Trigger shapes
        std::vector<Math::AABB> m_AABBs;
        std::vector<Math::OBB> m_OBBs;
        std::vector<Math::Sphere> m_spheres;
    };
} 
