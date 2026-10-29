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
		
        /// @brief Updates the component.
        void FixedUpdateImpl() override;
		
        //void ActiveChangedImpl(const bool p_active) override;
		
        void RenderUIImpl() override;
    
    };
} 
