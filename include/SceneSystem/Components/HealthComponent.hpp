#pragma once

#include <SceneSystem/Component.hpp>

namespace Droplet::Scene
{
class HealthComponent : public Component
{
public:
    [[nodiscard]] std::string_view GetTypeName() const override { return "HealthComponent"; }

    [[nodiscard]] float GetHealth() const { return m_health; }
    [[nodiscard]] float GetMaxHealth() const { return m_maxHealth; }

	void SetHealth(float p_health) { m_health = p_health; }
	void SetMaxHealth(float p_maxHealth) { m_maxHealth = p_maxHealth; }

    void TakeDamage(float p_amount);
    void Heal(float p_amount);

protected:
	void StartImpl() override;
	void UpdateImpl() override;
	void RenderUIImpl() override;
	nlohmann::json SerializeImpl() override;
	void DeserializeImpl(const nlohmann::json p_compJson) override;

private:
    float m_health = 100.0f;
    float m_maxHealth = 100.0f;
};
}