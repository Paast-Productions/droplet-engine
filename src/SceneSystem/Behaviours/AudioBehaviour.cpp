#include "SceneSystem/Behaviours/AudioBehaviour.hpp"
#include "Time.hpp"
#include "Editor/ResourceSlot.hpp"

#include <ImGui/imgui.h>
#include <ImGui/imgui_stdlib.h>

#include <algorithm>
#include <cmath>

namespace Droplet::Scene
{
    void AudioBehaviour::PlayTrack(const std::string &p_name, float p_fadeSeconds)
    {
        // Skip if track is already playing
        if (m_currentTrackName == p_name && 
            Audio::AudioSystem::GetInstance().GetPlaybackState(m_currentId) == Audio::PlaybackState::Playing)
        {
            return;
        }
        
        auto track = FindTrack(p_name);
        if (!track || !track->handle.Get()) return;
        
        Audio::PlaybackConfig config;
        config.busName = m_busName;
        config.looping = track->looping;
        config.pitch = track->pitch;
        config.stereoPan = track->stereoPan;
        float peakDb = std::clamp(track->volumeDb + m_targetVolumeDb, 
            Audio::C_VOLUME_DECIBEL_MIN, Audio::C_VOLUME_DECIBEL_MAX);
        
        auto&  audio = Audio::AudioSystem::GetInstance();
        
        // If a fade-out is happening -> Stop it
        if (m_fadingOutId != Audio::C_INVALID_PLAYBACK_ID)
        {
            audio.Stop(m_fadingOutId);
            m_fadingOutId = Audio::C_INVALID_PLAYBACK_ID;
        }
        
        if (p_fadeSeconds > 0.0f)
        {
            m_fadingOutId = m_currentId;
            config.volumeDb = Audio::C_VOLUME_DECIBEL_MIN;
            m_currentId = audio.Play2D(track->handle.Get(), config);
            
            m_fadeDuration = p_fadeSeconds;
            m_fadeElapsed = 0.0f;
        }
        else
        {
            if (m_currentId != Audio::C_INVALID_PLAYBACK_ID) audio.Stop(m_currentId);
            config.volumeDb = peakDb;
            m_currentId = audio.Play2D(track->handle.Get(), config);
            
            m_fadeDuration = 0.0f;
            m_fadeElapsed = 0.0f;
        }
        
        m_currentTrackName = p_name;
    }

    void AudioBehaviour::StopTracks(float p_fadeSeconds)
    {
        auto& audio = Audio::AudioSystem::GetInstance();
        
        if (p_fadeSeconds > 0.0f && m_currentId != Audio::C_INVALID_PLAYBACK_ID)
        {
            if (m_fadingOutId != Audio::C_INVALID_PLAYBACK_ID) audio.Stop(m_fadingOutId);
            m_fadingOutId = m_currentId;
            m_currentId = Audio::C_INVALID_PLAYBACK_ID;
            m_fadeDuration = p_fadeSeconds;
            m_fadeElapsed = 0.0f;
        }
        else
        {
            if (m_currentId != Audio::C_INVALID_PLAYBACK_ID) audio.Stop(m_currentId);
            if (m_fadingOutId != Audio::C_INVALID_PLAYBACK_ID) audio.Stop(m_fadingOutId);
            m_currentId = Audio::C_INVALID_PLAYBACK_ID;
            m_fadingOutId = Audio::C_INVALID_PLAYBACK_ID;
            m_fadeDuration = 0.0f;
        }
        
        m_currentTrackName.clear();
    }

    void AudioBehaviour::SetTargetVolumeDb(float p_volumeDb)
    {
        m_targetVolumeDb = std::clamp(p_volumeDb, Audio::C_VOLUME_DECIBEL_MIN, Audio::C_VOLUME_DECIBEL_MAX);
        if (m_fadeDuration <= 0.0f && m_currentId != Audio::C_INVALID_PLAYBACK_ID)
        {
            Audio::AudioSystem::GetInstance().SetPlaybackVolumeDb(m_currentId, m_targetVolumeDb);
        }
    }

    void AudioBehaviour::PreUpdateImpl()
    {
        if (m_fadeDuration <= 0.0f) return;
        
        m_fadeElapsed += Time::Get().GetDeltaTime();
        float t = std::clamp(m_fadeElapsed / m_fadeDuration, 0.0f, 1.0f);
        
        auto& audio = Audio::AudioSystem::GetInstance();
        
        // Fade stepping
        if (m_currentId != Audio::C_INVALID_PLAYBACK_ID)
        {
            float inDb = std::lerp(Audio::C_VOLUME_DECIBEL_MIN, m_targetVolumeDb, t);
            audio.SetPlaybackVolumeDb(m_currentId, inDb);
        }
        
        if (m_fadingOutId != Audio::C_INVALID_PLAYBACK_ID)
        {
            float outDb = std::lerp(m_targetVolumeDb, Audio::C_VOLUME_DECIBEL_MIN, t);
            audio.SetPlaybackVolumeDb(m_fadingOutId, outDb);
        }
        
        if (t >= 1.0f)
        {
            if (m_fadingOutId != Audio::C_INVALID_PLAYBACK_ID)
            {
                audio.Stop(m_fadingOutId);
                m_fadingOutId = Audio::C_INVALID_PLAYBACK_ID;
            }
            m_fadeDuration = 0.0f;
        }
    }

    void AudioBehaviour::OnDeactivate()
    {
        StopTracks(0.0f);
    }

    void AudioBehaviour::RenderUIImpl()
    {
        ImGui::PushID(this);
        
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
            }
            ImGui::EndDragDropTarget();
        }
        
        if (ImGui::SliderFloat("##VolumeSlider", &m_targetVolumeDb, Audio::C_VOLUME_DECIBEL_MIN, Audio::C_VOLUME_DECIBEL_MAX, "%.2f"))
        {
            SetTargetVolumeDb(m_targetVolumeDb);
        }
        ImGui::SameLine(); ImGui::Text(ICON_LC_VOLUME_2);
        
        ImGui::SeparatorText("Tracks");
        
        if (ImGui::Button("Add Track", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
        {
            m_tracks.push_back(AudioTrack{ .name = "Track " + std::to_string(m_tracks.size()) });
        }
        
        // --- Track List ---
        int removeIndex = -1;
        for (int32_t i = 0; i < static_cast<int32_t>(m_tracks.size()); i++)
        {
            AudioTrack& track = m_tracks[i];
            ImGui::PushID(i);
            
            std::string headerLabel = track.name + "##TrackHeader";
            bool open = ImGui::CollapsingHeader(headerLabel.c_str());
            if (open)
            {
                // Track name
                ImGui::InputText("Name", &track.name);
                
                // --- Resource Drag & Drop ---
                GUID newGuid = C_INVALID_GUID;
                std::string newName;
    
                if (ImGui::BeginTable("Sound Drop Zone", 2, ImGuiTableFlags_SizingStretchProp))
                {
                    ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                    ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

                    if (ResourceSlot(
                        "Sound",
                        track.soundGuid,
                        track.soundName,
                        newGuid,
                        newName,
                        StaticResourceType::Sound))
                    {
                        if (newGuid != C_INVALID_GUID)
                        {
                            track.handle = ResourceManager::GetInstance().LoadResource<SoundResource>(newGuid);
                        }
            
                        track.soundGuid = newGuid;
                        track.soundName = newName;
                    }
        
                    ImGui::EndTable();
                }
                
                // Parameters
                ImGui::SliderFloat("##VolumeSlider", &track.volumeDb, Audio::C_VOLUME_DECIBEL_MIN, Audio::C_VOLUME_DECIBEL_MAX, "%.2f");
                ImGui::SameLine(); ImGui::Text(ICON_LC_VOLUME_2);
	
                ImGui::SliderFloat("##StereoPanSlider", &track.stereoPan, Audio::C_STEREO_PAN_LEFT, Audio::C_STEREO_PAN_RIGHT);
                ImGui::SameLine(); ImGui::Text(ICON_LC_HEADPHONES);

                ImGui::SliderFloat("##PitchSlider", &track.pitch, Audio::C_PITCH_MIN, Audio::C_PITCH_MAX, " % .2f");
                ImGui::SameLine(), ImGui::Text(ICON_LC_AUDIO_WAVEFORM);
                
                ImGui::Checkbox("Looping", &track.looping);
                
                // Preview
                if (ImGui::Button(ICON_LC_PLAY "##Preview")) PlayTrack(track.name);
                ImGui::SameLine();
                if (ImGui::Button((ICON_LC_SQUARE))) StopTracks();
                ImGui::SameLine();
                if (ImGui::Button("Remove")) removeIndex = i;
            }
            
            ImGui::PopID();
        }
        
        if (removeIndex != -1)
        {
            if (m_tracks[removeIndex].name == m_currentTrackName) StopTracks();
            m_tracks.erase(m_tracks.begin() + removeIndex);
        }
        
        ImGui::PopID();
    }

    AudioTrack * AudioBehaviour::FindTrack(const std::string &p_name)
    {
        auto it = std::find_if(m_tracks.begin(), m_tracks.end(), [&p_name](const AudioTrack& track)
        {
            return track.name == p_name;
        });
        return (it != m_tracks.end()) ? &(*it) : nullptr;
    }
}
