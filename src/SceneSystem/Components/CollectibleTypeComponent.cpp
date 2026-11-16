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
    return m_itemId;
}

void Droplet::Scene::CollectibleTypeComponent::SetAmount(int p_amount)
{
    m_amount = p_amount;
}

int CollectibleTypeComponent::GetAmount() const
{
	return m_amount;
}

void Droplet::Scene::CollectibleTypeComponent::StartImpl()
{
}

void Droplet::Scene::CollectibleTypeComponent::UpdateImpl()
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
        SetCollectibleType(m_collectibleType);
    }


    ImGui::Separator();

    const char *itemNames[] =
    {
        "Wood",
        "Stone",
        "Iron",
        "Metal",
        "Eel",
        "Copper",
        "Tin",
        "Bronze",
        "Lead",
        "Broken Engine Part",
        "Repaired Engine Part"
    };

    Game::ItemId itemIds[] =
    {
        Game::Items::Wood,
        Game::Items::Stone,
        Game::Items::Iron,
        Game::Items::Metal,
        Game::Items::Eel,
        Game::Items::Copper,
        Game::Items::Tin,
        Game::Items::Bronze,
        Game::Items::Lead,
        Game::Items::BrokenEnginePart,
        Game::Items::RepairedEnginePart
    };

    int currentItem = -1;

    for (int i = 0; i < IM_ARRAYSIZE(itemIds); ++i)
    {
        if (m_itemId == itemIds[i])
        {
            currentItem = i;
            break;
        }
    }

    if (ImGui::Combo(
        "Item",
        &currentItem,
        itemNames,
        IM_ARRAYSIZE(itemNames)))
    {
        if (currentItem >= 0)
        {
            m_itemId = itemIds[currentItem];
        }
    }
}

nlohmann::json Droplet::Scene::CollectibleTypeComponent::SerializeImpl()
{
    return {
            {"collectibleType", m_collectibleType},
            {"itemId", m_itemId.value},
            {"amount", m_amount}
    };
}

void Droplet::Scene::CollectibleTypeComponent::DeserializeImpl(const nlohmann::json p_compJson)
{
	if (p_compJson.contains("collectibleType"))
	{
		m_collectibleType = p_compJson["collectibleType"].get<CollectibleType>();
	}
	if (p_compJson.contains("itemId"))
	{
		m_itemId = Game::ItemId(p_compJson["itemId"].get<int>());
	}
	if (p_compJson.contains("amount"))
	{
		m_amount = p_compJson["amount"].get<int>();
	}
}
