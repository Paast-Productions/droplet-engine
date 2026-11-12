#pragma once
#include <SceneSystem/Component.hpp>
#include "Audio/AudioSystem.hpp"

namespace Droplet::Scene
{
	/// @brief 3D audio listener.
	class AudioListenerComponent : public Component
	{
	public:
	    AudioListenerComponent();
	    ~AudioListenerComponent() override;
	    
		[[nodiscard]] std::string_view GetTypeName() const override { return "AudioListenerComponent"; }

	    // --- Setters ---
	    Audio::ListenerID GetListenerId() const { return m_listenerId; }
	    void SetListenerId(Audio::ListenerID id) { m_listenerId = id; }
	    
	    // --- Spatial Getters ---
	    [[nodiscard]] glm::vec3 GetWorldPosition() const;
	    [[nodiscard]] glm::vec3 GetForward() const;
	    [[nodiscard]] glm::vec3 GetUp() const;
	    [[nodiscard]] glm::vec3 GetVelocity() const;

	protected:
		void RenderUIImpl() override;

		nlohmann::json SerializeImpl() override;
		void DeserializeImpl(nlohmann::json p_compJson) override;

	private:
		Audio::ListenerID m_listenerId = Audio::C_INVALID_LISTENER_ID;
	};
}