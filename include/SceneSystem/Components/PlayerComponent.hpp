#pragma once

#include <SceneSystem/Component.hpp>
#include <SceneSystem/Behaviours/PlayerBehaviour.hpp>

namespace Droplet::Scene
{
    /// @brief Component identifying a scene Node as the player.
    ///
    /// Provides access to the PlayerBehaviour responsible for managing
    /// persistent player data. Maintains a weak reference to avoid
    /// taking ownership of the behaviour.
    class PlayerComponent : public Component
    {
    public:
        /// @brief Gets the component's registered type name.
        /// @return The type name of the component.
        [[nodiscard]] std::string_view GetTypeName() const override { return "PlayerComponent"; }

        /// @brief Gets a weak reference to the associated PlayerBehaviour.
        /// @return A weak pointer to the PlayerBehaviour, if available.
        [[nodiscard]] std::weak_ptr<PlayerBehaviour> GetPlayerBehaviour() { return m_playerBehaviour; }

        /// @brief Gets a shared reference to the PlayerBehaviour for Lua bindings.
        ///
        /// Locks the internally stored weak pointer, allowing Lua scripts
        /// to access the behaviour while the returned shared pointer exists.
        /// @return A shared pointer to the PlayerBehaviour, or nullptr
        /// if the behaviour no longer exists.
        [[nodiscard]] std::shared_ptr<PlayerBehaviour> GetPlayerBehaviourLua() { return m_playerBehaviour.lock(); }

    protected:
        /// @brief Initializes the component and associates it with the player behaviour.
        void StartImpl() override;

        /// @brief Updates the component.
        void UpdateImpl() override;

        /// @brief Renders the component's properties in the editor.
        void RenderUIImpl() override;

        /// @brief Serializes the component's data.
        /// @return A JSON object containing the serialized data.
        nlohmann::json SerializeImpl() override;

        /// @brief Restores the component's data from JSON.
        /// @param p_compJson The JSON object containing the serialized data.
        void DeserializeImpl(const nlohmann::json p_compJson) override;

    private:
        std::weak_ptr<PlayerBehaviour> m_playerBehaviour; /// Reference to the associated player behaviour.
    };
}