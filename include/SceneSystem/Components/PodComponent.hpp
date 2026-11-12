#pragma once
#include <SceneSystem/Component.hpp>
#include <string_view>
#include <memory>
#include <SceneSystem/Scene.hpp>

namespace Droplet::Scene
{
	class PodBehaviour;

	class PodComponent : public Component
	{
	public:
		~PodComponent();

		[[nodiscard]] std::string_view GetTypeName() const override { return "PodComponent"; }

		std::weak_ptr<Node> GetPlayerNode() const { return m_playerNode.GetTarget(); }
		std::weak_ptr<Node> GetDroneNode() const { return m_droneNode.GetTarget(); }

	protected:
		void StartImpl() override;

		void RenderUIImpl() override;

		[[nodiscard]] nlohmann::json SerializeImpl();
		void DeserializeImpl(const nlohmann::json p_compJson);
		void ResolveReferences(SerializationContext &p_context) override;

	private:
		std::weak_ptr<PodBehaviour> m_podBehaviour;
		SceneReference<Node> m_playerNode;
		SceneReference<Node> m_droneNode;
	};
}