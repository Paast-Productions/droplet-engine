#include "SceneSystem/Behaviours/AudioBehaviour.hpp"
#include "Time.hpp"

#include <algorithm>
#include <cmath>

namespace Droplet::Scene
{
    void AudioBehaviour::RegisterTrack(const std::string &p_name, GUID p_soundGuid)
    {
        if (p_soundGuid == C_INVALID_GUID) return;
        m_tracks[p_name] = ResourceManager::GetInstance().LoadResource<SoundResource>(p_soundGuid);
    }

    void AudioBehaviour::PlayTrack(const std::string &p_name, float p_fadeSeconds)
    {
        // Skip if track is already playing
        if (m_currentTrackName == p_name && 
            Audio::AudioSystem::GetInstance().GetPlaybackState(m_currentId) == Audio::PlaybackState::Playing)
        {
            return;
        }
        
        auto it = m_tracks.find(p_name);
        if (it == m_tracks.end() || !it->second.Get()) return;
        
        auto&  audio = Audio::AudioSystem::GetInstance();
        
        // If a fade-out is happening -> Stop it
        if (m_fadingOutId != Audio::C_INVALID_PLAYBACK_ID)
        {
            audio.Stop(m_fadingOutId);
            m_fadingOutId = Audio::C_INVALID_PLAYBACK_ID;
        }
        
        Audio::PlaybackConfig config;
        config.busName = m_busName;
        config.looping = true;
        
        if (p_fadeSeconds > 0.0f)
        {
            m_fadingOutId = m_currentId;
            config.volumeDb = Audio::C_VOLUME_DECIBEL_MIN;
            m_currentId = audio.Play2D(it->second.Get(), config);
            
            m_fadeDuration = p_fadeSeconds;
            m_fadeElapsed = 0.0f;
        }
        else
        {
            if (m_currentId != Audio::C_INVALID_PLAYBACK_ID) audio.Stop(m_currentId);
            config.volumeDb = m_targetVolumeDb;
            m_currentId = audio.Play2D(it->second.Get(), config);
            
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
    }
}
