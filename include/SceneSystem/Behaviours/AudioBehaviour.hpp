#pragma once 
#include "SceneSystem/Behaviour.hpp"
#include "Audio/AudioSystem.hpp"
#include "Resource/Types/SoundResource.hpp"
#include "Resource/StaticResourceHandle.hpp"

#include <unordered_map>
#include <string>

namespace Droplet::Scene
{
    struct AudioTrack
    {
        std::string name = "New Track";
        GUID soundGuid = C_INVALID_GUID;
        std::string soundName = "";
        StaticResourceHandle<SoundResource> handle;
        
        float volumeDb = Audio::C_VOLUME_DECIBEL_DEFAULT;
        float stereoPan = Audio::C_STEREO_PAN_MIDDLE;
        float pitch = Audio::C_PITCH_DEFAULT;
        bool looping = false;
    };
    
    class AudioBehaviour : public Behaviour
    {
    public:
        std::string_view GetTypeName() override { return "AudioBehaviour"; }
        bool IsSingleton() override { return true; }
        
        void PlayTrack(const std::string& p_name, float p_fadeSeconds = 0.0f);
        void StopTracks(float p_fadeSeconds = 0.0f);
        
        void SetTargetVolumeDb(float p_volumeDb);
        
        void SetBusName(const std::string& p_busName) { m_busName = p_busName; }
        
    protected:
        void PreUpdateImpl() override;
        void OnDeactivate() override;
        
        void RenderUIImpl() override;
        
        nlohmann::json SerializeImpl() override;
        void DeserializeImpl(const nlohmann::json p_behJson) override;
        
    private:
        AudioTrack* FindTrack(const std::string& p_name);
        
        std::vector<AudioTrack> m_tracks;
        std::string m_currentTrackName;
        std::string m_busName = "Master";
        
        Audio::PlaybackID m_currentId   = Audio::C_INVALID_PLAYBACK_ID;
        Audio::PlaybackID m_fadingOutId = Audio::C_INVALID_PLAYBACK_ID;
        
        float m_currentTrackPeakDb  = Audio::C_VOLUME_DECIBEL_DEFAULT;
        float m_currentInDb         = Audio::C_VOLUME_DECIBEL_DEFAULT;
        float m_fadeOutStartDb      = Audio::C_VOLUME_DECIBEL_DEFAULT;
        
        float m_fadeDuration    = 0.0f;
        float m_fadeElapsed     = 0.0f;
    };
}