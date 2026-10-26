#pragma once
#include <SceneSystem/Component.hpp>


namespace Droplet::Scene 
{
	/// @brief Right now just outline class to start work with later
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
		bool m_is2D = false;
		bool m_is3D = false;

		// This is a temp for just testing the UI, need to be replaced with the mesh position later
		glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 0.0f);
	};
}