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

Droplet::Scene::AudioEmitterComponent::AudioEmitterComponent()
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
    SoundResource* sound = m_resourceHandle.Get();
    if (!sound)
    {
        Debug::Log(Debug::LogType::Warning, "AudioEmitterComponent: Failed to play. Unable to acquire sound resource.");
        return;
    }
    
    if (IsPlaying()) Stop();
    m_playbackId = Audio::AudioSystem::GetInstance().Play3D(this, sound);
    m_playbackState = Audio::PlaybackState::Playing;
    m_isDirty = false;
}

void AudioEmitterComponent::Pause()
{
    if (m_playbackId != Audio::C_INVALID_PLAYBACK_ID)
    {
        Audio::AudioSystem::GetInstance().Pause(m_playbackId);
        m_playbackState = Audio::PlaybackState::Paused;
    }
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
    return m_playbackId != Audio::C_INVALID_PLAYBACK_ID && m_playbackState == Audio::PlaybackState::Playing;
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
    Transform& transformComp = GetOwner()->GetTransform();
    return transformComp.GetPosition(Transform::Space::World);
}

glm::vec3 AudioEmitterComponent::GetVelocity() const
{
    std::shared_ptr<PhysicsComponent> physComp = GetOwner()->GetComponent<PhysicsComponent>();
    if (physComp) return physComp->GetVelocity();
    return glm::vec3(0.0f, 0.0f, 0.0f);
}

void AudioEmitterComponent::OnPlaybackFinished()
{
    m_playbackState = Audio::PlaybackState::Inactive;
    m_playbackId = Audio::C_INVALID_PLAYBACK_ID;
}

void Droplet::Scene::AudioEmitterComponent::StartImpl()
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
            ResourceType::Sound))
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
    
    // --- Play/Pause ---
	ImVec2 buttonSize = ImVec2(30, 30);
	if (ImGui::Button(ICON_LC_PLAY "##PlaySoundButton",buttonSize))
	{
        Play();
	}
	ImGui::SameLine();
	if (ImGui::Button(ICON_LC_SQUARE "##StopSoundButton", buttonSize))
	{
		Stop();
	}
    // TODO: Add a way to pause?
    // TODO: Add a way to seek?
    
    
    // --- General Settings ---
	ImGui::SeparatorText("General Settings" ICON_LC_SETTINGS);
    // TODO: Make non-linear
	if (ImGui::SliderFloat("Volume " ICON_LC_VOLUME_2, &m_volumeDb, -60.0f, 12.0f, "%.2f"))
	{
	    m_isDirty = true;
	}
	if (ImGui::SliderFloat("Pitch", &m_pitch, 0.1f, 4.0f, " % .2f")) m_isDirty = true;
	if (ImGui::Checkbox("Looping", &m_looping)) m_isDirty = true;

    // --- Spatial Settings ---
    ImGui::SeparatorText("Spatial Settings" ICON_LC_SETTINGS);
	ImGui::DragFloatRange2("Sound Distance", &m_minDistance, &m_maxDistance, 
	    0.1f, 0.0f, 100.0f, "Min: %.1f", "Max: %.1f");
    
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
	ImGui::SeparatorText("Attenuation Curve");
	if (ImGui::Combo("##AttenuationCurve", &current, items, IM_COUNTOF(items)))
	{
		m_attenuationCurve = curves[current];
	}
    
    ImGui::PopID();
}

nlohmann::json Droplet::Scene::AudioEmitterComponent::SerializeImpl()
{
	
	nlohmann::json json;

    json["soundGuid"] = m_soundGuid;
    json["soundName"] = m_soundName;
    json["busName"] = m_busName;
    
	json["volumeDb"] = m_volumeDb;
    json["pitch"] = m_pitch;
	json["looping"] = m_looping;
    
    json["minDistance"] = m_minDistance;
    json["maxDistance"] = m_maxDistance;
	json["attenuationCurve"] = m_attenuationCurve;

	return json;
}

void Droplet::Scene::AudioEmitterComponent::DeserializeImpl(nlohmann::json p_compJson)
{
    if (p_compJson.contains("soundGuid")) m_soundGuid = p_compJson["soundGuid"];
    if (p_compJson.contains("soundName")) m_soundName = p_compJson["soundName"];
    if (p_compJson.contains("busName")) m_busName = p_compJson["busName"];
    
	if (p_compJson.contains("volumeDb")) m_volumeDb = p_compJson["volumeDb"];
	if (p_compJson.contains("pitch")) m_pitch = p_compJson["pitch"];
	if (p_compJson.contains("looping")) m_looping = p_compJson["looping"];
    
	if (p_compJson.contains("minDistance")) m_minDistance = p_compJson["minDistance"];
	if (p_compJson.contains("maxDistance")) m_maxDistance = p_compJson["maxDistance"];
	if (p_compJson.contains("attenuationCurve")) m_attenuationCurve = p_compJson["attenuationCurve"];
}
	

