#pragma once

#include <Component.hpp>
#include <SceneSystem/Bounds/DefaultNodeBounds.hpp>
//#include <SceneSystem/Node.hpp>

namespace Droplet::Scene
{
    class TriggerComponent : public Component
    {
    public:
        TriggerComponent() = default;
        
        std::string_view GetTypeName() const override { return "PhysicsComponent"; }
    private:
        
    protected:
        /// @brief Starts the component.
        void StartImpl() override;
		
        /// @brief Checks every fixed frame when to call trigger functions.
        void FixedUpdateImpl() override;
		
        /// @brief Internal rendering function for the component's UI.
        ///
        /// Overloaded to provide a default UI for the ScriptComponent that
        /// displays the script path. Derived components can override this
        /// function to implement their own UI rendering logic.
        void RenderUIImpl() override;
    
    };
} 
