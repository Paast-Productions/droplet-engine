#pragma once

#include <SceneSystem/Component.hpp>
#include <SceneSyste/Items/BaseItems.hpp>

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

        void SetCollectibleType(const std::string &p_collectibleType);
		[[nodiscard]] std::string &GetCollectibleType() const;

    protected:
        void StartImpl() override;
        void UpdateImpl() override;
        void RenderUIImpl() override;

    private:
		CollectibleType m_collectibleType;
        ItemRegistry &m_items;
	    int m_collectibleAmount;
    };
}