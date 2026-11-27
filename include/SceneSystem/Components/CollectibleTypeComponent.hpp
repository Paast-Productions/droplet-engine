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
        ~CollectibleTypeComponent() = default;

        std::string_view GetTypeName() const override { return "CollectibleTypeComponent"; }

        void SetCollectibleType(CollectibleType p_collectibleType);
		[[nodiscard]] CollectibleType GetCollectibleType() const;

		void SetItemId(Game::ItemId p_itemId);
		[[nodiscard]] Game::ItemId GetItemId() const;

		void SetAmount(int p_amount);
		[[nodiscard]] int GetAmount() const;

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
		CollectibleType m_collectibleType = CollectibleType::Fish;
		Game::ItemId m_itemId = Game::InvalidItemId;
	    int m_amount = 0;
    };
}