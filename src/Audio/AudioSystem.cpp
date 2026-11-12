#include "Audio/AudioSystem.hpp"
#include "Resource/Types/SoundResource.hpp"
#include <dalia/dalia.h>

using namespace Droplet::Audio;

void LogDaliaError(const std::string& message, dalia::Result error)
{   
    std::string messageStr = message + std::string(dalia::GetResultString(error));
    Droplet::Debug::Log(Droplet::Debug::LogType::Error, messageStr);
}

bool Droplet::Audio::AudioSystem::Initialize()
{
	dalia::EngineConfig config;
	dalia::Result res = m_daliaEngine->Initialize(config);
	if (res != dalia::Result::Ok) return false;
    
	m_mixer.Initialize(*m_daliaEngine);
    
    return true;
}

void Droplet::Audio::AudioSystem::Shutdown()
{
	m_daliaEngine->Shutdown();
    // m_mixer.Shutdown(); // Don't we need this?
}

void AudioSystem::Update()
{
    ProcessFinishedPlaybacks(); // Clean up finished handles
    
    // Sync listeners
    for (auto* listener : m_activeListeners)
    {
        auto pos = listener->GetWorldPosition();
        auto fwd = listener->GetForwardVector();
        auto up = listener->GetUpVector();
        auto vel = listener->GetVelocity();
        
        dalia::Listener3DAttributes attributes {
            .position = { pos.x, pos.y, pos.z },
            .forward = { fwd.x, fwd.y, fwd.z },
            .up = { up.x, up.y, up.z },
            .velocity = { vel.x, vel.y, vel.z }
        };
        m_daliaEngine->SetListener3DAttributes(listener->GetIndex(), attributes);
    }
    
    // Sync players
    for (auto* player : m_activePlayers)
    {
        PlaybackID id = emitter->GetPlaybackID();
        auto it = m_activePlaybacks.find(id);
        if (it != m_activePlaybacks.end())
        {
            if (emitter->IsDirty())
            {
                m_daliaEngine->SetPlaybackVolumeDb(it->second, emitter->GetVolumeDb);
                emitter->ClearDirtyFlag();
            }
        }
    }
    
    // Sync emitters
    for (auto* emitter : m_activeEmitters)
    {
        PlaybackID id = emitter->GetPlaybackID();
        auto it = m_activePlaybacks.find(id);
        if (it != m_activePlaybacks.end())
        {
            // Update spatial properties
            auto pos = emitter->GetWorldPosition();
            m_daliaEngine->SetPlaybackPosition(it->second, { pos.x, pos.y, pos.z });
            
            auto vel = emitter->GetVelocity();
            m_daliaEngine->SetPlaybackVelocity(it->second, { vel.x, vel.y, vel.z });
            
            if (emitter->IsDirty())
            {
                m_daliaEngine->SetPlaybackVolumeDb(it->second, emitter->GetVolumeDb);
                emitter->ClearDirtyFlag();
            }
        }
    }
    
    m_daliaEngine.Update();
}

std::unique_ptr<Droplet::SoundResource> AudioSystem::LoadSound(const std::filesystem::path &p_path, SoundType p_type,
                                                               std::function<void(bool)> p_onLoad)
{
    dalia::SoundType daliaSoundType;
    if (p_type == SoundType::Resident) daliaSoundType = dalia::SoundType::Resident;
    else daliaSoundType = dalia::SoundType::Stream;
    
    dalia::SoundHandle handle = dalia::INVALID_SOUND_HANDLE;
    dalia::Result res = m_daliaEngine->LoadSoundAsync(handle, daliaSoundType, p_path.generic_string().c_str(), 
        [p_onLoad](uint32_t reqId, dalia::Result result)
    {
        if (p_onLoad)
        {
            if (result == dalia::Result::Ok) p_onLoad(true);
            else p_onLoad(false);
        }
    });
    
    if (res == dalia::Result::Ok) return std::make_unique<SoundResource>(handle);
    return nullptr;
}

PlaybackID AudioSystem::Play3D(AudioEmitterComponent *emitter, SoundResource *sound)
{
    PlaybackID newId = m_nextPlaybackId++;
    dalia::PlaybackHandle daliaHandle = dalia::INVALID_PLAYBACK_HANDLE;
    
    // Callback that fires once the playback has finished
    auto exitCallback = [this, newId] (dalia::PlaybackHandle handle, dalia::PlaybackExitCondition condition)
    {
        m_finishedPlaybacks.push_back(newId);
    };
    
    dalia::Result res = m_daliaEngine->CreatePlayback(daliaHandle, sound->GetHandle(), exitCallback);
    if (res != dalia::Result::Ok)
    {
        LogDaliaError("Failed to play 3D sound.", res);
        return C_INVALID_PLAYBACK_ID;
    }
    
    m_daliaEngine->SetPlaybackSpatial(daliaHandle, true);
    m_daliaEngine->SetPlaybackVolumeDb(daliaHandle, emitter->GetVolumeDb());
    
    m_daliaEngine->PlayPlayback(daliaHandle);
    
    m_activePlaybacks[newId] = daliaHandle;
    return newId;
}

void AudioSystem::Pause(PlaybackID id)
{
    auto it = m_activePlaybacks.find(id);
    if (it != m_activePlaybacks.end()) m_daliaEngine->PausePlayback(it->second);
}

void AudioSystem::Stop(PlaybackID id)
{
    auto it = m_activePlaybacks.find(id);
    if (it != m_activePlaybacks.end()) m_daliaEngine->StopPlayback(it->second);
}

void AudioSystem::ProcessFinishedPlaybacks()
{
    for (PlaybackID id : m_finishedPlaybacks)
    {
        m_activePlaybacks.erase(id);
        
        // Notify players
        if (auto* player : m_activePlayers) if (player->GetPlaybackID() == id) player->NotifyStopped();
        
        // Notify emitters
        if (auto* emitter : m_activeEmitters) if (emitter->GetPlaybackID() == id) emitter->NotifyStopped();
    }
    
}
