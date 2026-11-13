
#include "GraphicsPCH.h"
#include "CollectibleTypeComponent.hpp"

using namespace Droplet::Scene;

void CollectibleTypeComponent::SetCollectibleType(CollectibleType p_collectibleType)
{
    m_collectibleType = p_collectibleType;
}

CollectibleType CollectibleTypeComponent::GetCollectibleType() const
{
    return m_collectibleType;
}

void CollectibleTypeComponent::SetItemId(Game::ItemId p_itemId)
{
	m_itemId = p_itemId;
}

Droplet::Game::ItemId CollectibleTypeComponent::GetItemId() const
{
    return Game::ItemId();
}

void Droplet::Scene::CollectibleTypeComponent::SetAmount(int p_amount)
{
    m_amount = p_amount;
}

int CollectibleTypeComponent::GetAmount() const
{
	return m_amount;
}

void CollectibleTypeComponent::StartImpl()
{
}

void CollectibleTypeComponent::UpdateImpl()
{
}

void CollectibleTypeComponent::RenderUIImpl()
{

    const char *collectibleTypes[] =
    {
        "Fish",
        "Upgrade",
        "Broken Part"
    };

    int currentType = static_cast<int>(m_collectibleType);

    if (ImGui::Combo(
        "Collectible Type",
        &currentType,
        collectibleTypes,
        IM_ARRAYSIZE(collectibleTypes)))
    {
        m_collectibleType = static_cast<CollectibleType>(currentType);
    }

}
