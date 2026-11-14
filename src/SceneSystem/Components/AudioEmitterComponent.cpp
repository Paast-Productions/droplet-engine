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

using namespace Droplet::Scene;

AudioEmitterComponent::AudioEmitterComponent()
{
    Audio::AudioSystem::GetInstance().RegisterEmitter(this);
}

AudioEmitterComponent::~AudioEmitterComponent()
{
    Stop();
    Audio::AudioSystem::GetInstance().UnregisterEmitter(this);
}

void AudioEmitterComponent::Play()
{
    if (m_playbackId != Audio::C_INVALID_PLAYBACK_ID) Stop();
    if (!IsActiveSelf()) return;
    
    SoundResource* sound = m_resourceHandle.Get();
    if (!sound)
    {
        Debug::Log(Debug::LogType::Warning, "AudioEmitterComponent: Failed to play. Unable to acquire sound resource.");
        return;
    }
    
    m_playbackId = Audio::AudioSystem::GetInstance().Play3D(this, sound);
    m_isDirty = false;
}

void AudioEmitterComponent::Pause()
{
    if (m_playbackId == Audio::C_INVALID_PLAYBACK_ID) return;
    if (!IsActiveSelf()) return;
    
    Audio::AudioSystem::GetInstance().Pause(m_playbackId);
}

void AudioEmitterComponent::Resume()
{
    if (m_playbackId == Audio::C_INVALID_PLAYBACK_ID) Stop();
    if (!IsActiveSelf()) return;
    
    Audio::AudioSystem::GetInstance().Resume(m_playbackId);
}

void AudioEmitterComponent::Stop()
{
    if (m_playbackId != Audio::C_INVALID_PLAYBACK_ID)
    {
        Audio::AudioSystem::GetInstance().Stop(m_playbackId);
    }
}

bool AudioEmitterComponent::IsPlaying() const
{
    return m_playbackId != Audio::C_INVALID_PLAYBACK_ID && 
        Audio::AudioSystem::GetInstance().GetPlaybackState(m_playbackId) == Audio::PlaybackState::Playing;
}

void AudioEmitterComponent::SetStartTimeSeconds(double p_startTimeSeconds)
{
    m_startTimeSeconds = p_startTimeSeconds;
}

void AudioEmitterComponent::SetVolumeDb(float p_volume)
{
    m_volumeDb = p_volume;
    m_isDirty = true;
}

void AudioEmitterComponent::SetPitch(float p_pitch)
{
    m_pitch = p_pitch;
    m_isDirty = true;
}

void AudioEmitterComponent::SetLooping(bool p_looping)
{
    m_looping = p_looping;
    m_isDirty = true;
}

void AudioEmitterComponent::SetMinMaxDistance(float p_min, float p_max)
{
    m_minDistance = p_min;
    m_maxDistance = p_max;
    m_isDirty = true;
}

void AudioEmitterComponent::SetAttenuationCurve(dalia::AttenuationCurve p_curve)
{
    m_attenuationCurve = p_curve;
    m_isDirty = true;
}

void AudioEmitterComponent::SetBusName(const std::string &p_busName)
{
    m_busName = p_busName;
    m_isDirty = true;
}

glm::vec3 AudioEmitterComponent::GetWorldPosition() const
{
    if (!GetOwner())
    {
        return glm::vec3(0.0f, 0.0f, 0.0f);
	}

    Transform &transformComp = GetOwner()->GetTransform();
    return transformComp.GetPosition(Transform::Space::World);
}

glm::vec3 AudioEmitterComponent::GetVelocity() const
{
    if (!GetOwner())
    {
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }

    std::shared_ptr<PhysicsComponent> physComp = GetOwner()->GetComponent<PhysicsComponent>();
    if (!physComp)
    {
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }
        
    return physComp->GetVelocity();
}

void AudioEmitterComponent::SetSoundGUID(GUID p_guid)
{
    m_resourceHandle = ResourceManager::GetInstance().LoadResource<SoundResource>(p_guid);
    if (MetaEntry entry; ResourceManager::GetInstance().GetResourceMetaEntry(p_guid, entry))
    {
        m_soundName = entry.displayName;
    }
}

void AudioEmitterComponent::OnPlaybackFinished()
{
    m_playbackId = Audio::C_INVALID_PLAYBACK_ID;
}

void AudioEmitterComponent::StartImpl()
{

}

void AudioEmitterComponent::RenderUIImpl()
{
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
    // const std::string preview = m_busName.empty() ? "Drop Audio Bus Here" : m_busName;
    // ImGui::Button(preview.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 32.0f));
    //
    // if (ImGui::BeginDragDropTarget())
    // {
    //     const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("AUDIO_BUS");
    //     
    //     if (payload != nullptr && payload->IsDelivery() && payload->DataSize == sizeof(std::uint64_t))
    //     {
    //         std::uint64_t draggedBusID = 0;
    //         std::memcpy(&draggedBusID, payload->Data, sizeof(draggedBusID));
    //
    //         Audio::AudioBus *draggedBus = Audio::AudioSystem::GetInstance().FindAudioBus(draggedBusID);
    //         if (draggedBus) m_busName = draggedBus->GetName();
    //     }
    //     ImGui::EndDragDropTarget();
    // }
    // if (!Audio::AudioSystem::GetInstance().HasAudioBusWithName(m_busName)) m_busName = "Drop Audio Bus Here";
    
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

	if (ImGui::SliderFloat("##PitchSlider", &m_pitch, Audio::C_PITCH_MIN, Audio::C_PITCH_MAX, " % .2f"))
	{
	    m_isDirty = true;
	}
    ImGui::SameLine(), ImGui::Text(ICON_LC_AUDIO_WAVEFORM);


    // --- Spatial Settings ---
    if (ImGui::Checkbox("Doppler", &m_doppler)) m_isDirty = true;
    
	dalia::AttenuationCurve curves[] = { dalia::AttenuationCurve::InverseSquare,dalia::AttenuationCurve::Linear, dalia::AttenuationCurve::Quadratic };
	const char *items[] = { "Inverse Square", "Linear", "Quadratic" };
	int current = 0;
	for (int i = 0; i < IM_COUNTOF(items); ++i)
	{
		if (curves[i] == m_attenuationCurve)
		{
			current = i;
			break;
		}
	}
    
    ImGui::Spacing();
	ImGui::Text("Attenuation"); 
	if (ImGui::Combo("##AttenuationCurve", &current, items, IM_COUNTOF(items)))
	{
		m_attenuationCurve = curves[current];
	    m_isDirty = true;
	}
    
    if (ImGui::DragFloatRange2("##MinMaxDistance", &m_minDistance, &m_maxDistance, 
    0.1f, 0.0f, 10000.0f, "Min: %.1f", "Max: %.1f"))
	{
	    m_isDirty = true;
	}
    
    ImGui::PopID();
}

void AudioEmitterComponent::ActiveChangedImpl(const bool p_active)
{
    if (!p_active) Stop();
}

nlohmann::json AudioEmitterComponent::SerializeImpl()
{
	nlohmann::json json;

    json["soundGuid"] = m_soundGuid;
    json["busName"] = m_busName;
    
    json["startTimeSeconds"] = m_startTimeSeconds;
	json["volumeDb"] = m_volumeDb;
    json["pitch"] = m_pitch;
	json["looping"] = m_looping;
    
    json["minDistance"] = m_minDistance;
    json["maxDistance"] = m_maxDistance;
	json["attenuationCurve"] = m_attenuationCurve;
    json["doppler"] = m_doppler;

	return json;
}

void AudioEmitterComponent::DeserializeImpl(nlohmann::json p_compJson)
{
    if (p_compJson.contains("soundGuid")) m_soundGuid = p_compJson["soundGuid"];
    SetSoundGUID(m_soundGuid);
    if (p_compJson.contains("busName")) m_busName = p_compJson["busName"];
    
    if (p_compJson.contains("startTimeSeconds")) m_startTimeSeconds = p_compJson["startTimeSeconds"];
	if (p_compJson.contains("volumeDb")) m_volumeDb = p_compJson["volumeDb"];
	if (p_compJson.contains("pitch")) m_pitch = p_compJson["pitch"];
	if (p_compJson.contains("looping")) m_looping = p_compJson["looping"];
    
	if (p_compJson.contains("minDistance")) m_minDistance = p_compJson["minDistance"];
	if (p_compJson.contains("maxDistance")) m_maxDistance = p_compJson["maxDistance"];
	if (p_compJson.contains("attenuationCurve")) m_attenuationCurve = p_compJson["attenuationCurve"];
    if (p_compJson.contains("doppler")) m_doppler = p_compJson["doppler"];
}
	

