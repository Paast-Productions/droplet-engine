#pragma once

#include <SceneSystem/Behaviour.hpp>
#include "PlayerData.hpp"

namespace Droplet::Scene
{
    class PlayerBehaviour : public Behaviour
    {
    public:
        PlayerBehaviour() = default;

        std::string_view GetTypeName() override
        {
            return "PlayerBehaviour";
        }

        [[nodiscard]] const Droplet::Game::PlayerData &GetData() const
        {
            return m_data;
        }

    protected:
        [[nodiscard]] nlohmann::json SerializeImpl() override;

        void DeserializeImpl(const nlohmann::json p_behJson) override;

    private:
        Droplet::Game::PlayerData m_data;
    };
}