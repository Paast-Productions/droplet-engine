#pragma once

#include <SceneSystem/Component.hpp>
#include <SceneSystem/Behaviours/PlayerBehaviour.hpp>

namespace Droplet::Scene
{
    class PlayerComponent : public Component
    {
    public:
        [[nodiscard]] std::string_view GetTypeName() const override { return "PlayerComponent"; }
        [[nodiscard]] std::weak_ptr<PlayerBehaviour> GetPlayerBehaviour() { return m_playerBehaviour; }
        [[nodiscard]] std::shared_ptr<PlayerBehaviour> GetPlayerBehaviourLua() { return m_playerBehaviour.lock(); }

    protected:
        void StartImpl() override;
        void UpdateImpl() override;
        void RenderUIImpl() override;
        nlohmann::json SerializeImpl() override;
        void DeserializeImpl(const nlohmann::json p_compJson) override;

	private:
		std::weak_ptr<PlayerBehaviour> m_playerBehaviour;
    };
}