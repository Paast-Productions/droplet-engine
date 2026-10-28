#include "HealthComponent.hpp"

#include <algorithm>
#include <cmath>

#include <ImGui/imgui.h>

namespace Droplet::Scene
{
    void HealthComponent::StartImpl()
    {
        m_maxHealth = std::max(0.0f, m_maxHealth);
        m_health = std::clamp(m_health, 0.0f, m_maxHealth);
    }

    void HealthComponent::UpdateImpl()
    {
        // No per-frame logic is currently required.
    }

    void HealthComponent::TakeDamage(float p_amount)
    {
        if (!std::isfinite(p_amount) || p_amount <= 0.0f)
        {
            return;
        }

        m_health = std::max(0.0f, m_health - p_amount);
    }

    void HealthComponent::Heal(float p_amount)
    {
        if (!std::isfinite(p_amount) || p_amount <= 0.0f)
        {
            return;
        }

        m_health = std::min(m_maxHealth, m_health + p_amount);
    }

    void HealthComponent::RenderUIImpl()
    {
        ImGui::DragFloat("Health", &m_health, 1.0f, 0.0f, m_maxHealth);
        ImGui::DragFloat("Max Health", &m_maxHealth, 1.0f, 0.0f);

        // Keep health valid if either value is changed in the editor.
        m_maxHealth = std::max(0.0f, m_maxHealth);
        m_health = std::clamp(m_health, 0.0f, m_maxHealth);

        const float fraction = m_maxHealth > 0.0f
            ? m_health / m_maxHealth
            : 0.0f;

        ImGui::ProgressBar(fraction, ImVec2(-1.0f, 0.0f));
    }

    nlohmann::json HealthComponent::SerializeImpl()
    {
        return {
            {"health", m_health},
            {"maxHealth", m_maxHealth}
        };
    }

    void HealthComponent::DeserializeImpl(const nlohmann::json p_compJson)
    {
        m_maxHealth = p_compJson.value("maxHealth", 100.0f);
        m_maxHealth = std::max(0.0f, m_maxHealth);

        m_health = p_compJson.value("health", m_maxHealth);
        m_health = std::clamp(m_health, 0.0f, m_maxHealth);
    }
}