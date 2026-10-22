#pragma once
#include <SceneSystem/Component.hpp>


namespace Droplet::Scene
{
	class AudioListenerComponent : public Component
	{
	public:
		AudioListenerComponent();

		[[nodiscard]] std::string_view GetTypeName() const override { return "AudioListenerComponent"; }


	protected:
		void StartImpl() override;
		void UpdateImpl(float deltaTime) override;
		void RenderUIImpl() override;

		nlohmann::json SerializeImpl() override;
		void DeserializeImpl(nlohmann::json p_compJson) override;

	private:

	};
}