#pragma once
#include <SceneSystem/Component.hpp>


namespace Droplet::Scene 
{
	class AudioEmitterComponent : public Component
	{
	public:
		AudioEmitterComponent();

		[[nodiscard]] std::string_view GetTypeName() const override { return "AudioEmitterComponent"; }


	protected:
		void StartImpl() override;
		void UpdateImpl(float deltaTime) override;
		void RenderUIImpl() override;

		nlohmann::json SerializeImpl() override;
		void DeserializeImpl(nlohmann::json p_compJson) override;

	private:

		float m_volume = 0;
		bool m_looping = false;


	};
}