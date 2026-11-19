#include "AudioEmitterComponent.hpp"
#include "ImGui/imgui.h"
#include <SceneSystem/Node.hpp>
#include <SceneSystem/Scene.hpp>
#include <Audio/AudioSystem.hpp>
#include <Audio/AudioBus.hpp>
#include <SceneSystem/Behaviour.hpp>

#include "PhysicsComponent.hpp"
#include "Editor/DragDropFields.hpp"
#include "Editor/ResourceSlot.hpp"

Droplet::Scene::AudioPlayerComponent::AudioPlayerComponent()
{

}

Droplet::Scene::AudioPlayerComponent::~AudioPlayerComponent()
{
	Stop();
	Audio::AudioSystem::GetInstance().UnregisterPlayerComponent(this);
}

void Droplet::Scene::AudioPlayerComponent::Play()
{
	if (!IsActiveSelf()) return;
	// Stop playback if calling play on an active playback instance
	if (m_playbackId != Audio::C_INVALID_PLAYBACK_ID || 
		Audio::AudioSystem::GetInstance().GetPlaybackState(m_playbackId) != Audio::PlaybackState::Inactive) Stop();
    
	SoundResource* sound = m_resourceHandle.Get();
	if (!sound)
	{
		Debug::Log(Debug::LogType::Warning, "AudioPlayerComponent: Failed to play. Unable to acquire sound resource.");
		return;
	}
    
	m_playbackId = Audio::AudioSystem::GetInstance().Play(this, sound);
	m_isDirty = false;
}

void Droplet::Scene::AudioPlayerComponent::Pause()
{
	if (m_playbackId == Audio::C_INVALID_PLAYBACK_ID) return;
	if (!IsActiveSelf()) return;
    
	Audio::AudioSystem::GetInstance().Pause(m_playbackId);
}

void Droplet::Scene::AudioPlayerComponent::Resume()
{
	if (m_playbackId == Audio::C_INVALID_PLAYBACK_ID) Stop();
	if (!IsActiveSelf()) return;
    
	Audio::AudioSystem::GetInstance().Resume(m_playbackId);
}

void Droplet::Scene::AudioPlayerComponent::Stop()
{
	if (m_playbackId != Audio::C_INVALID_PLAYBACK_ID)
	{
		Audio::AudioSystem::GetInstance().Stop(m_playbackId);
	}
}

bool Droplet::Scene::AudioPlayerComponent::IsPlaying() const
{
    return m_playbackId != Audio::C_INVALID_PLAYBACK_ID && 
        Audio::AudioSystem::GetInstance().GetPlaybackState(m_playbackId) == Audio::PlaybackState::Playing;
}

void Droplet::Scene::AudioPlayerComponent::SetStartTimeSeconds(double p_startTimeSeconds)
{
	m_startTimeSeconds = p_startTimeSeconds;
}

void Droplet::Scene::AudioPlayerComponent::SetVolumeDb(float p_volume)
{
	m_volumeDb = std::clamp(p_volume, Audio::C_VOLUME_DECIBEL_MIN, Audio::C_VOLUME_DECIBEL_MAX);
	m_isDirty = true;
}

void Droplet::Scene::AudioPlayerComponent::SetStereoPan(float p_stereoPan)
{
	m_stereoPan = std::clamp(p_stereoPan, Audio::C_STEREO_PAN_LEFT, Audio::C_STEREO_PAN_RIGHT);
}

void Droplet::Scene::AudioPlayerComponent::SetPitch(float p_pitch)
{
	m_pitch = std::clamp(p_pitch, Audio::C_PITCH_MIN, Audio::C_PITCH_MAX);
	m_isDirty = true;
}

void Droplet::Scene::AudioPlayerComponent::SetLooping(bool p_looping)
{
	m_looping = p_looping;
	m_isDirty = true;
}

void Droplet::Scene::AudioPlayerComponent::SetSound(GUID p_guid)
{
	m_resourceHandle = ResourceManager::GetInstance().LoadResource<SoundResource>(p_guid);
	if (MetaEntry entry; ResourceManager::GetInstance().GetResourceMetaEntry(p_guid, entry))
	{
		m_soundName = entry.displayName;
	}
}

void Droplet::Scene::AudioPlayerComponent::OnPlaybackFinished()
{
	m_playbackId = Audio::C_INVALID_PLAYBACK_ID;
}

void Droplet::Scene::AudioPlayerComponent::OnBusInvalidation()
{
	m_busName = "Master";
}

void Droplet::Scene::AudioPlayerComponent::StartImpl()
{
}

void Droplet::Scene::AudioPlayerComponent::RenderUIImpl()
{
#ifdef _EDITOR
	ImGui::PushID(this);
    
    // --- Resource Drag & Drop ---
    GUID newGuid = C_INVALID_GUID;
    std::string newName;
    
    if (ImGui::BeginTable("Drop Zone", 2, ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

        if (ResourceSlot(
            "Sound",
            m_soundGuid,
            m_soundName,
            newGuid,
            newName,
            StaticResourceType::Sound))
        {
            if (newGuid != C_INVALID_GUID)
            {
                m_resourceHandle = ResourceManager::GetInstance().LoadResource<SoundResource>(newGuid);
            }
            
            m_soundName = newName;
            m_soundGuid = newGuid;
        }
        
        ImGui::EndTable();
    }

    ImGui::Text(ICON_LC_FILE_VOLUME " %.2f s", Audio::AudioSystem::GetInstance().GetSoundLenghtInSeconds(m_resourceHandle.Get()));
    
    ImGui::Spacing();
    
    // --- Bus Drag & Drop ---
     const std::string preview = m_busName.empty() ? "Drop Audio Bus Here" : m_busName;
     ImGui::Button(preview.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 32.0f));
    
     if (ImGui::BeginDragDropTarget())
     {
         const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(std::string(Editor::PAYLOAD_AUDIO_BUS).c_str());
         
         if (payload != nullptr && payload->IsDelivery() && payload->DataSize == sizeof(std::uint64_t))
         {
             std::uint64_t draggedBusID = 0;
             std::memcpy(&draggedBusID, payload->Data, sizeof(draggedBusID));
         	
             Audio::AudioBus *draggedBus = Audio::AudioSystem::GetInstance().GetMixer().FindAudioBus(draggedBusID);
             if (draggedBus) m_busName = draggedBus->GetName();
             m_routingChanged = true;
         }
         ImGui::EndDragDropTarget();
     }

    
    // --- State ---
    Audio::PlaybackState currentState = Audio::AudioSystem::GetInstance().GetPlaybackState(m_playbackId);
    
    ImVec2 buttonSize = ImVec2(20, 20);
    if (currentState == Audio::PlaybackState::Inactive) {
        if (ImGui::Button(ICON_LC_PLAY "##PlaySoundButton",buttonSize)) Play();
    }
    else if (currentState == Audio::PlaybackState::Paused)
    {
        if (ImGui::Button(ICON_LC_PLAY "##ResumeSoundButton", buttonSize)) Resume();
    }
    else 
    {
        if (ImGui::Button(ICON_LC_PAUSE "##PauseSoundButton",buttonSize)) Pause();
    }
    
	ImGui::SameLine();
	if (ImGui::Button(ICON_LC_SQUARE "##StopSoundButton", buttonSize)) Stop();
    
    ImGui::SameLine();
    switch (currentState) {
        case Audio::PlaybackState::Inactive: ImGui::TextColored({1.0f, 0.2f, 0.2f, 1.0f}, "Inactive");	break;
        case Audio::PlaybackState::Playing:  ImGui::TextColored({0.0f, 1.0f, 0.0f, 1.0f}, "Playing");	break;
        case Audio::PlaybackState::Paused:   ImGui::TextColored({1.0f, 1.0f, 0.0f, 1.0f}, "Paused");	break;
    }
    
    ImGui::InputDouble("Start Time (s)", &m_startTimeSeconds, 0.1);
    if (ImGui::Checkbox("Looping", &m_looping)) m_isDirty = true;
    
    ImGui::Spacing();
    
	if (ImGui::SliderFloat("##VolumeSlider", &m_volumeDb, Audio::C_VOLUME_DECIBEL_MIN, Audio::C_VOLUME_DECIBEL_MAX, "%.2f"))
	{
	    m_isDirty = true;
	}
    ImGui::SameLine(); ImGui::Text(ICON_LC_VOLUME_2);
	
	if (ImGui::SliderFloat("##StereoPanSlider", &m_stereoPan, Audio::C_STEREO_PAN_LEFT, Audio::C_STEREO_PAN_RIGHT))
	{
		m_isDirty = true;
	}
	ImGui::SameLine(); ImGui::Text(ICON_LC_HEADPHONES);

	if (ImGui::SliderFloat("##PitchSlider", &m_pitch, Audio::C_PITCH_MIN, Audio::C_PITCH_MAX, " % .2f"))
	{
	    m_isDirty = true;
	}
    ImGui::SameLine(); ImGui::Text(ICON_LC_AUDIO_WAVEFORM);
    
    ImGui::PopID();
#endif
}

void Droplet::Scene::AudioPlayerComponent::ActiveChangedImpl(const bool p_active)
{
    if (p_active) Audio::AudioSystem::GetInstance().RegisterPlayerComponent(this);
    else
    {
        Stop();
        Audio::AudioSystem::GetInstance().UnregisterPlayerComponent(this);
    }
}

nlohmann::json Droplet::Scene::AudioPlayerComponent::SerializeImpl()
{
	nlohmann::json json;

	json["soundGuid"] = m_soundGuid;
	json["busName"] = m_busName;
    
	json["startTimeSeconds"] = m_startTimeSeconds;
	json["volumeDb"] = m_volumeDb;
	json["stereoPan"] = m_stereoPan;
	json["pitch"] = m_pitch;
	json["looping"] = m_looping;

	return json;
}

void Droplet::Scene::AudioPlayerComponent::DeserializeImpl(nlohmann::json p_compJson)
{
	if (p_compJson.contains("soundGuid")) m_soundGuid = p_compJson["soundGuid"];
	SetSound(m_soundGuid);
	if (p_compJson.contains("busName")) m_busName = p_compJson["busName"];
	if (!Audio::AudioSystem::GetInstance().GetMixer().HasAudioBusWithName(m_busName)) m_busName = "Master";
    
	if (p_compJson.contains("startTimeSeconds")) m_startTimeSeconds = p_compJson["startTimeSeconds"];
	if (p_compJson.contains("volumeDb")) m_volumeDb = p_compJson["volumeDb"];
	if (p_compJson.contains("stereoPan")) m_stereoPan = p_compJson["stereoPan"];
	if (p_compJson.contains("pitch")) m_pitch = p_compJson["pitch"];
	if (p_compJson.contains("looping")) m_looping = p_compJson["looping"];
}
