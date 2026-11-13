#pragma once

#include <SceneSystem/Component.hpp>
#include <SceneSystem/Items/BaseItems.hpp>

namespace Droplet::Scene
{
    enum CollectibleType
    {
        Fish,
        Upgrade,
        BrokenPart
    };

    /// @brief Allows a node to be interacted with, invoking its attached Lua script.
    class CollectibleTypeComponent : public Component
    {
    public:
		CollectibleTypeComponent() = default;

        std::string_view GetTypeName() const override { return "CollectibleTypeComponent"; }

        void SetCollectibleType(CollectibleType p_collectibleType);
		[[nodiscard]] CollectibleType GetCollectibleType() const;

		void SetItemId(Game::ItemId p_itemId);
		[[nodiscard]] Game::ItemId GetItemId() const;

		void SetAmount(int p_amount);
		[[nodiscard]] int GetAmount() const;

    protected:
        void StartImpl() override;
        void UpdateImpl() override;
        void RenderUIImpl() override;

    private:
		CollectibleType m_collectibleType = CollectibleType::Fish;
		Game::ItemId m_itemId = Game::InvalidItemId;
	    int m_amount = 0;
    };
}