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
        
        /// @brief Internal serialization function for the component's state.
        /// 
        /// Overloaded by derived components to implement their own serialization logic.
        /// @return A JSON object representing the component's state.
        [[nodiscard]] nlohmann::json SerializeImpl() override;

        /// @brief Internal deserialization function for the component's state.
        /// 
        /// Overloaded by derived components to implement their own deserialization logic.
        /// @param p_compJson A JSON object containing the component's state.
        void DeserializeImpl([[maybe_unused]] const nlohmann::json p_compJson) override;
    
    private:
        struct Trigger
        {
            glm::vec3 offset; // Offset from node position
        };
        struct TriggerAABB : Trigger
        {
            Math::AABB aabb;
            void Draw(const glm::vec4 p_color, const bool p_depth = true, const bool p_wire = true) const
            {
                Graphics::Debug::DebugDraw::DrawBox(aabb, p_color, p_depth, p_wire);
            }
        };
        struct TriggerOBB : Trigger
        {
            Math::OBB obb;
            void Draw(const glm::vec4 p_color, const bool p_depth = true, const bool p_wire = true) const
            {
                Graphics::Debug::DebugDraw::DrawBox(obb, p_color, p_depth, p_wire);
            }
        };
        struct TriggerSphere : Trigger
        {
            Math::Sphere sphere;
            void Draw(const glm::vec4 p_color, const bool p_depth = true, const bool p_wire = true) const
            {
                if (p_wire)
                {
                    Graphics::Debug::DebugDraw::DrawCircle(sphere.center, glm::vec3(1.0f, 0.0f, 0.0f), sphere.radius, p_color, 16, p_depth, true);
                    Graphics::Debug::DebugDraw::DrawCircle(sphere.center, glm::vec3(0.0f, 1.0f, 0.0f), sphere.radius, p_color, 16, p_depth, true);
                    Graphics::Debug::DebugDraw::DrawCircle(sphere.center, glm::vec3(0.0f, 0.0f, 1.0f), sphere.radius, p_color, 16, p_depth, true);
                    Graphics::Debug::DebugDraw::DrawCircle(sphere.center, glm::vec3(1.0f, 0.0f, 1.0f), sphere.radius, p_color, 16, p_depth, true);
                    Graphics::Debug::DebugDraw::DrawCircle(sphere.center, glm::vec3(1.0f, 0.0f, -1.0f), sphere.radius, p_color, 16, p_depth, true);
                }
                else
                {
                    Graphics::Debug::DebugDraw::DrawSphere(sphere,p_color, 1, p_depth);
                }
            }
        };
        
#ifdef _EDITOR
        /// @brief Add a type of shape
        /// @param p_currentShape Chosen shape type
        void AddShape(int &p_currentShape);
        
        /// @brief Lists shapes and option to remove shape
        /// @param p_currentShape Chosen shape type
        /// @return Index of selected shape
        int ListShapes(const int p_currentShape);
#endif
        
        bool m_isColliding = false;
        std::vector<std::weak_ptr<Node>> m_collidingNodes;
        
        // Trigger shapes
        std::vector<TriggerAABB> m_AABBs;
        std::vector<TriggerOBB> m_OBBs;
        std::vector<TriggerSphere> m_spheres;
    };
} 
