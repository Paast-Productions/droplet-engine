#pragma once
#include <SceneSystem/Component.hpp>
#include <string_view>
#include <memory>
#include <SceneSystem/Scene.hpp>

namespace Droplet::Scene
{
	class TrackerComponent : public Component
	{
	public:
		[[nodiscard]] std::string_view GetTypeName() const override { return "TrackerComponent"; }

		void SetTrackedTarget(std::weak_ptr<Node> p_target) { m_trackedNode.SetTarget(p_target.lock()); }
		[[nodiscard]] std::weak_ptr<Node> GetTrackedTarget() const { return m_trackedNode.GetTarget(); }

	protected:
		void LateUpdateImpl() override;

		void RenderUIImpl() override;

		[[nodiscard]] nlohmann::json SerializeImpl();
		void DeserializeImpl(const nlohmann::json p_compJson);
		void ResolveReferences(SerializationContext &p_context) override;

	private:
		SceneReference<Node> m_trackedNode;

		bool m_trackPosX = true;
		bool m_trackPosY = true;
		bool m_trackPosZ = true;

		bool m_trackRotX = true;
		bool m_trackRotY = true;
		bool m_trackRotZ = true;
	};
}