#pragma once

#include <SceneSystem/Behaviour.hpp>
#include "PlayerData.hpp"

#include <memory>

namespace Droplet::Scene
{
    class Node;

    class PlayerBehaviour : public Behaviour
    {
    public:
        PlayerBehaviour() = default;

        std::string_view GetTypeName() override
        {
            return "PlayerBehaviour";
        }

        /// @brief Sets the Node representing the player.
        /// @param p_player The player Node.
        void SetPlayer(const std::shared_ptr<Node> &p_player)
        {
            m_player = p_player;
        }

        /// @brief Gets the player Node, if it still exists.
        [[nodiscard]] std::shared_ptr<Node> GetPlayer() const
        {
            return m_player.lock();
        }

        [[nodiscard]] const Droplet::Game::PlayerData &GetData() const
        {
            return m_data;
        }

		[[nodiscard]] float GetMaxHealth()
		{
            return m_data.maxHealth;
		}

    protected:
        [[nodiscard]] nlohmann::json SerializeImpl() override;

        void DeserializeImpl(const nlohmann::json p_behJson) override;

    private:
        std::weak_ptr<Node> m_player;
        Droplet::Game::PlayerData m_data;
    };
}