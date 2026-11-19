#include "Audio/AudioSystem.hpp"
#include "EffectRegistry.hpp"
#include "Effects/ReverbEffect.hpp"
#include "Effects/LowPassEffect.hpp"
#include "Effects/HighPassEffect.hpp"
#include "SceneSystem/Components/AudioListenerComponent.hpp"
#include "Resource/Types/SoundResource.hpp"
#include "SceneSystem/Components/AudioPlayerComponent.hpp"
#include "SceneSystem/Components/AudioEmitterComponent.hpp"
#include "SceneSystem/Components/AudioListenerComponent.hpp"

#include <dalia/dalia.h>

#include "Node.hpp"

using namespace Droplet::Audio;

void LogDaliaError(const std::string& message, dalia::Result error)
{   
    std::string messageStr = message + std::string(dalia::GetResultString(error));
    Droplet::Debug::Log(Droplet::Debug::LogType::Error, messageStr);
}

bool Droplet::Audio::AudioSystem::Initialize()
{
    m_daliaEngine = std::make_unique<dalia::Engine>();
    
	dalia::EngineConfig config;
    config.coordinateSystem = dalia::CoordinateSystem::RightHanded;
    config.listenerCapacity = 4;
    config.streamCapacity = 64;
    config.voiceCapacity = 128;
    config.busCapacity = 128;
	dalia::Result res = m_daliaEngine->Initialize(config);
	if (res != dalia::Result::Ok) return false;
    RegisterEffects();
    m_mixer.Initialize(*m_daliaEngine);
    
#if defined(_GAME) 
    m_isGameMode = true;
#else
    m_isGameMode = false;
#endif
    
    m_daliaEngine->SetListenerActive(C_EDITOR_LISTENER_ID, !m_isGameMode);
    
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
    
    // Sync emitters
    for (auto* player : m_playerComponents)
    {
        if (!player->IsActiveSelf()) continue;
        
        PlaybackID id = player->GetPlaybackID();
        auto it = m_playbacks.find(id);
        if (it != m_playbacks.end())
        {
            dalia::PlaybackHandle daliaHandle = it->second.handle;
            
            if (player->IsDirty())
            {
                m_daliaEngine->SetPlaybackVolumeDb(daliaHandle, player->GetVolumeDb());
                m_daliaEngine->SetPlaybackStereoPan(daliaHandle, player->GetStereoPan());
                m_daliaEngine->SetPlaybackRate(daliaHandle, player->GetPitch());
                m_daliaEngine->SetPlaybackLooping(daliaHandle, player->IsLooping());
                player->ClearDirtyFlag();
            }
            
            if (player->RoutingChanged())
            {
                if (!m_mixer.HasAudioBusWithName(player->GetBusName())) player->OnBusInvalidation();
                m_daliaEngine->RoutePlayback(daliaHandle, player->GetBusName().c_str());
                player->ClearDirtyFlag();
            }
        }
    }
    
    // Sync emitters
    for (auto* emitter : m_emitterComponents)
    {
        if (!emitter->IsActiveSelf()) continue;
        
        PlaybackID id = emitter->GetPlaybackID();
        auto it = m_playbacks.find(id);
        if (it != m_playbacks.end())
        {
            dalia::PlaybackHandle daliaHandle = it->second.handle;
            // Update spatial properties
            auto pos = emitter->GetWorldPosition();
            m_daliaEngine->SetPlaybackPosition(daliaHandle, { pos.x, pos.y, pos.z });
            
            auto vel = emitter->GetVelocity();
            m_daliaEngine->SetPlaybackVelocity(daliaHandle, { vel.x, vel.y, vel.z });
            
            if (emitter->IsDirty())
            {
                m_daliaEngine->SetPlaybackVolumeDb(daliaHandle, emitter->GetVolumeDb());
                m_daliaEngine->SetPlaybackRate(daliaHandle, emitter->GetPitch());
                m_daliaEngine->SetPlaybackLooping(daliaHandle, emitter->IsLooping());
                m_daliaEngine->SetPlaybackMinMaxDistance(daliaHandle, emitter->GetMinDistance(), emitter->GetMaxDistance());
                m_daliaEngine->SetPlaybackAttenuationCurve(daliaHandle, emitter->GetAttenuationCurve());
                emitter->ClearDirtyFlag();
            }
            
            if (emitter->RoutingChanged())
            {
                if (!m_mixer.HasAudioBusWithName(emitter->GetBusName())) emitter->OnBusInvalidation();
                m_daliaEngine->RoutePlayback(daliaHandle, emitter->GetBusName().c_str());
                emitter->ClearDirtyFlag();
            }
        }
    }
    
    // Sync listeners
    for (auto* listener : m_listenerComponents)
    {
        if (!listener->IsActiveSelf()) continue;
        
        auto pos = listener->GetWorldPosition();
        auto fwd = listener->GetForward();
        auto up = listener->GetUp();
        auto vel = listener->GetVelocity();
        
        dalia::Listener3DAttributes attributes {
            .position = { pos.x, pos.y, pos.z },
            .forward = { fwd.x, fwd.y, fwd.z },
            .up = { up.x, up.y, up.z },
            .velocity = { vel.x, vel.y, vel.z }
        };
        m_daliaEngine->SetListener3DAttributes(listener->GetListenerId(), attributes);
    }
    
    m_daliaEngine->Update();
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

void AudioSystem::UnloadSound(SoundResource *sound)
{
    if (!sound) return;
    m_daliaEngine->UnloadSound(sound->GetHandle());
}

double AudioSystem::GetSoundLenghtInSeconds(SoundResource *sound) const
{
    if (!sound) return 0.0;
    double lenghtInSeconds = 0.0;
    m_daliaEngine->GetSoundLength(sound->GetHandle(), lenghtInSeconds);
    return lenghtInSeconds;
}

PlaybackID AudioSystem::Play2D(SoundResource *p_sound, const PlaybackConfig &p_config)
{
    if (!p_sound || m_isGamePaused) return C_INVALID_PLAYBACK_ID;

    PlaybackID newId = m_nextPlaybackId++;
    dalia::PlaybackHandle daliaHandle = dalia::INVALID_PLAYBACK_HANDLE;

    auto exitCallback = [this, newId](dalia::PlaybackHandle, dalia::PlaybackExitCondition)
    {
        m_finishedPlaybacks.push_back(newId);
    };

    dalia::Result res = m_daliaEngine->CreatePlayback(daliaHandle, p_sound->GetHandle(), exitCallback);
    if (res != dalia::Result::Ok)
    {
        LogDaliaError("AudioSystem: Failed to play 2D sound.", res);
        return C_INVALID_PLAYBACK_ID;
    }

    m_daliaEngine->SetPlaybackSpatial(daliaHandle, false);

    const char* bus = m_mixer.HasAudioBusWithName(p_config.busName) ? p_config.busName.c_str() : "Master";
    res = m_daliaEngine->RoutePlayback(daliaHandle, bus);
    if (res != dalia::Result::Ok) LogDaliaError("AudioSystem: Failed to route 2D playback.", res);

    if (p_config.startTimeSeconds > 0.001) m_daliaEngine->SeekPlayback(daliaHandle, p_config.startTimeSeconds);
    m_daliaEngine->SetPlaybackVolumeDb(daliaHandle, std::clamp(p_config.volumeDb, C_VOLUME_DECIBEL_MIN, C_VOLUME_DECIBEL_MAX));
    m_daliaEngine->SetPlaybackStereoPan(daliaHandle, std::clamp(p_config.stereoPan, C_STEREO_PAN_LEFT, C_STEREO_PAN_RIGHT));
    m_daliaEngine->SetPlaybackRate(daliaHandle, std::clamp(p_config.pitch, C_PITCH_MIN, C_PITCH_MAX));
    m_daliaEngine->SetPlaybackLooping(daliaHandle, p_config.looping);

    m_daliaEngine->PlayPlayback(daliaHandle);
    m_playbacks[newId] = { .handle = daliaHandle, .state = PlaybackState::Playing };
    return newId;
}

void AudioSystem::SetPlaybackVolumeDb(PlaybackID p_id, float p_volumeDb)
{
    auto it = m_playbacks.find(p_id);
    if (it != m_playbacks.end())
    {
        float clamped = std::clamp(p_volumeDb, C_VOLUME_DECIBEL_MIN, C_VOLUME_DECIBEL_MAX);
        m_daliaEngine->SetPlaybackVolumeDb(it->second.handle, clamped);
    }
}

void AudioSystem::SetPlaybackPitch(PlaybackID p_id, float p_pitch)
{
    auto it = m_playbacks.find(p_id);
    if (it != m_playbacks.end())
    {
        float clamped = std::clamp(p_pitch, C_PITCH_MIN, C_PITCH_MAX);
        m_daliaEngine->SetPlaybackRate(it->second.handle, clamped);
    }
}

void AudioSystem::SetPlaybackStereoPan(PlaybackID p_id, float p_pan)
{
    auto it = m_playbacks.find(p_id);
    if (it != m_playbacks.end())
    {
        float clamped = std::clamp(p_pan, C_STEREO_PAN_LEFT, C_STEREO_PAN_RIGHT);
        m_daliaEngine->SetPlaybackStereoPan(it->second.handle, clamped);
    }
}

void AudioSystem::SetPlaybackLooping(PlaybackID p_id, bool p_looping)
{
    auto it = m_playbacks.find(p_id);
    if (it != m_playbacks.end())
    {
        m_daliaEngine->SetPlaybackLooping(it->second.handle, p_looping);
    }
}

bool AudioSystem::RoutePlayback(PlaybackID p_id, const std::string& p_busName)
{
    auto it = m_playbacks.find(p_id);
    if (it == m_playbacks.end()) return false;

    const char* bus = m_mixer.HasAudioBusWithName(p_busName) ? p_busName.c_str() : "Master";
    dalia::Result res = m_daliaEngine->RoutePlayback(it->second.handle, bus);
    if (res != dalia::Result::Ok)
    {
        LogDaliaError("AudioSystem: Failed to route playback.", res);
        return false;
    }
    return true;
}

void AudioSystem::RegisterPlayerComponent(Scene::AudioPlayerComponent *player)
{
    auto it = std::find(m_playerComponents.begin(), m_playerComponents.end(), player);
    if (it == m_playerComponents.end()) m_playerComponents.push_back(player);
}

void AudioSystem::UnregisterPlayerComponent(Scene::AudioPlayerComponent *player)
{
    auto it = std::find(m_playerComponents.begin(), m_playerComponents.end(), player);
    if (it != m_playerComponents.end())
    {
        auto playbackIt = m_playbacks.find(player->GetPlaybackID());
        if (playbackIt != m_playbacks.end())
        {
            m_daliaEngine->StopPlayback(playbackIt->second.handle);
        }
        
        m_playerComponents.erase(it);
        player->OnPlaybackFinished();
    }
}

void AudioSystem::RegisterEmitterComponent(Scene::AudioEmitterComponent *emitter)
{
    auto it = std::find(m_emitterComponents.begin(), m_emitterComponents.end(), emitter);
    if (it == m_emitterComponents.end()) m_emitterComponents.push_back(emitter);
}

void AudioSystem::UnregisterEmitterComponent(Scene::AudioEmitterComponent *emitter)
{
    auto it = std::find(m_emitterComponents.begin(), m_emitterComponents.end(), emitter);
    if (it != m_emitterComponents.end())
    {
        auto playbackIt = m_playbacks.find(emitter->GetPlaybackID());
        if (playbackIt != m_playbacks.end())
        {
            m_daliaEngine->StopPlayback(playbackIt->second.handle);
        }
        
        m_emitterComponents.erase(it);
        emitter->OnPlaybackFinished();
    }
}

void AudioSystem::RegisterListenerComponent(Scene::AudioListenerComponent *listener)
{
    if (listener->GetListenerId() != C_INVALID_LISTENER_ID) return;
    
    for (uint32_t i = 1; i < 4; i++)
    {
        if (!m_listeners[i].taken)
        {
            m_listeners[i].taken = true;
            m_listeners[i].active = listener->IsActiveSelf();
            listener->SetListenerId(i);
            m_listenerComponents.push_back(listener);
            
            // Scene masking
            Scene::Scene* scene = listener->GetOwner()->GetScene().get();
            m_sceneListenerMasks[scene] |= dalia::MakeListenerMask(i);
            
            if (m_isGameMode && listener->IsActiveSelf()) m_daliaEngine->SetListenerActive(i, true);
            return;
        }
    }
    Debug::Log(Debug::LogType::Error, "AudioSystem: Maximum number of listeners (3) are already in use.");
}

void AudioSystem::UnregisterListenerComponent(Scene::AudioListenerComponent *listener)
{
    ListenerID id = listener->GetListenerId();
    if (id == C_INVALID_LISTENER_ID) return;
    
    auto it = std::find(m_listenerComponents.begin(), m_listenerComponents.end(), listener);
    if (it != m_listenerComponents.end())
    {
        // Scene masking
        Scene::Scene* scene = listener->GetOwner()->GetScene().get();
        auto maskIt = m_sceneListenerMasks.find(scene);
        if (maskIt != m_sceneListenerMasks.end())
        {
            maskIt->second &= ~dalia::MakeListenerMask(id);
            if (maskIt->second == dalia::MASK_NONE) m_sceneListenerMasks.erase(maskIt); // Clean up if none remain
        }
        
        m_daliaEngine->SetListener3DAttributes(id, dalia::Listener3DAttributes{});
        m_daliaEngine->SetListenerActive(id, false);
        
        m_listeners[id] = ListenerData{};
        m_listenerComponents.erase(it);
        
        listener->SetListenerId(C_INVALID_LISTENER_ID);
    }
}

void AudioSystem::SetListenerActive(ListenerID p_id, bool p_active)
{
    if (p_id < C_LISTENERS_MAX && m_listeners[p_id].taken)
    {
        m_listeners[p_id].active = p_active;
        if (m_isGameMode) m_daliaEngine->SetListenerActive(p_id, p_active);
    }
}

PlaybackID AudioSystem::Play(Scene::AudioPlayerComponent *player, SoundResource *sound)
{
    if (m_isGamePaused) return C_INVALID_PLAYBACK_ID; // Game is paused -> do not allow new play calls
    
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
        LogDaliaError("Failed to play sound.", res);
        return C_INVALID_PLAYBACK_ID;
    }
    m_daliaEngine->SetPlaybackSpatial(daliaHandle, false);

    res = m_daliaEngine->RoutePlayback(daliaHandle, player->GetBusName().c_str());
    if (res != dalia::Result::Ok)
    {
        // Routing failed -> Fallback to master
        LogDaliaError("Failed to route playback.", res);
        player->OnBusInvalidation();
    }
    
    double seekTime = player->GetStartTimeSeconds();
    if (seekTime > 0.001) m_daliaEngine->SeekPlayback(daliaHandle, seekTime);
    m_daliaEngine->SetPlaybackVolumeDb(daliaHandle, player->GetVolumeDb());
    m_daliaEngine->SetPlaybackStereoPan(daliaHandle, player->GetStereoPan());
    m_daliaEngine->SetPlaybackRate(daliaHandle, player->GetPitch());
    m_daliaEngine->SetPlaybackLooping(daliaHandle, player->IsLooping());
    
    m_daliaEngine->PlayPlayback(daliaHandle);
    m_playbacks[newId] = { .handle = daliaHandle, .state = PlaybackState::Playing };
    return newId;
}

PlaybackID AudioSystem::PlaySpatial(Scene::AudioEmitterComponent *emitter, SoundResource *sound)
{
    if (m_isGamePaused) return C_INVALID_PLAYBACK_ID; // Game is paused -> do not allow new play calls
    
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

    res = m_daliaEngine->RoutePlayback(daliaHandle, emitter->GetBusName().c_str());
    if (res != dalia::Result::Ok)
    {
        // Routing failed -> Fallback to master
        LogDaliaError("Failed to route playback.", res);
        emitter->OnBusInvalidation();
    }
    
    // Make listener target only listeners in the same scene
    Scene::Scene* playerScene = emitter->GetOwner()->GetScene().get();
    dalia::ListenerMask mask = dalia::MASK_ALL_LISTENERS;
    auto it = m_sceneListenerMasks.find(playerScene);
    if (it != m_sceneListenerMasks.end()) mask = it->second | dalia::MASK_LISTENER_0;
    m_daliaEngine->SetPlaybackListenerMask(daliaHandle, mask);
    
    double seekTime = emitter->GetStartTimeSeconds();
    if (seekTime > 0.001) m_daliaEngine->SeekPlayback(daliaHandle, seekTime);
    m_daliaEngine->SetPlaybackVolumeDb(daliaHandle, emitter->GetVolumeDb());
    m_daliaEngine->SetPlaybackRate(daliaHandle, emitter->GetPitch());
    m_daliaEngine->SetPlaybackLooping(daliaHandle, emitter->IsLooping());
    m_daliaEngine->SetPlaybackMinMaxDistance(daliaHandle, emitter->GetMinDistance(), emitter->GetMaxDistance());
    m_daliaEngine->SetPlaybackAttenuationCurve(daliaHandle, emitter->GetAttenuationCurve());
    
    m_daliaEngine->PlayPlayback(daliaHandle);
    m_playbacks[newId] = { .handle = daliaHandle, .state = PlaybackState::Playing };
    return newId;
}

void AudioSystem::Pause(PlaybackID id)
{
    auto it = m_playbacks.find(id);
    if (it != m_playbacks.end()) 
    {
        it->second.state = PlaybackState::Paused;
        m_daliaEngine->PausePlayback(it->second.handle);
    }
}

void AudioSystem::Resume(PlaybackID id)
{
    auto it = m_playbacks.find(id);
    if (it != m_playbacks.end()) 
    {
        it->second.state = PlaybackState::Playing;
        if (!m_isGamePaused)
        {
            m_daliaEngine->PlayPlayback(it->second.handle);
        }
    }
}

void AudioSystem::Stop(PlaybackID id)
{
    auto it = m_playbacks.find(id);
    if (it != m_playbacks.end()) 
    {
        it->second.state = PlaybackState::Inactive;
        m_daliaEngine->StopPlayback(it->second.handle);
    }
}

PlaybackState AudioSystem::GetPlaybackState(PlaybackID id) const
{
    auto it = m_playbacks.find(id); 
    if (it != m_playbacks.end()) return it->second.state;
    return PlaybackState::Inactive;
}

void AudioSystem::SetMode(bool p_isGameMode)
{
    m_isGameMode = p_isGameMode;
    
    // Stop all active playbacks in dalia
    for (auto& [id, data] : m_playbacks)
    {
        m_daliaEngine->StopPlayback(data.handle);
    }
    m_playbacks.clear();
    
    for (auto* player : m_playerComponents) player->OnPlaybackFinished();
    for (auto* emitter : m_emitterComponents) emitter->OnPlaybackFinished();
    
    // Activate/deactivate listeners based on mode
    m_daliaEngine->SetListenerActive(C_EDITOR_LISTENER_ID, !p_isGameMode);
    for (uint32_t i = 1; i < 4; i++)
    {
        if (m_listeners[i].taken && m_listeners[i].active != false)
        {
            m_daliaEngine->SetListenerActive(i, p_isGameMode);
        }
    }
}

void AudioSystem::SetGamePaused(bool p_paused)
{
    m_isGamePaused = p_paused;
    
    for (auto& [id, data] : m_playbacks)
    {
        if (p_paused) m_daliaEngine->PausePlayback(data.handle);
        else
        {
            if (data.state == PlaybackState::Playing) m_daliaEngine->PlayPlayback(data.handle);
        }
    }   
}

void AudioSystem::UpdateEditorListener3DAttributes(const glm::vec3 &p_pos, const glm::vec3 &p_fwd,
                                                   const glm::vec3 &p_up, const glm::vec3 &p_vel)
{
    m_daliaEngine->SetListener3DAttributes(C_EDITOR_LISTENER_ID, { 
        .position = { p_pos.x, p_pos.y, p_pos.z },
        .forward = { p_fwd.x, p_fwd.y, p_fwd.z },
        .up = { p_up.x, p_up.y, p_up.z },
        .velocity = { p_vel.x, p_vel.y, p_vel.z }
    });
}

bool Droplet::Audio::AudioSystem::CreateEffect(dalia::EffectHandle &p_handle, dalia::BiquadParams& p_params)
{
    dalia::Result res;
    res = m_daliaEngine->CreateEffect(p_handle, p_params);
    if (res != dalia::Result::Ok)
    {
        std::cout << "failed to creaate handle" << std::endl;
        return false;
    }

    return true;
}

bool Droplet::Audio::AudioSystem::AttachEffect(dalia::EffectHandle &p_effect, const std::string& p_busname, uint32_t p_slot)
{
    dalia::Result res;
    res = m_daliaEngine->AttachEffect(p_effect, p_busname.c_str(), p_slot);

    if (res == dalia::Result::ExpiredHandle)
    {
        std::cout << "Expired" << std::endl;
    }

    if (res == dalia::Result::InvalidHandle)
    {
        std::cout << "Invalid" << std::endl;
    }

    if (res != dalia::Result::Ok)
    {
        return false;
    }

    return true;
}

void AudioSystem::ProcessFinishedPlaybacks()
{
    for (PlaybackID id : m_finishedPlaybacks)
    {
        m_playbacks.erase(id);
        
        // Notify player components
        for (auto* player : m_playerComponents)
        {
            if (player->GetPlaybackID() == id) player->OnPlaybackFinished();
        }
        // Notify emitter components
        for (auto* emitter : m_emitterComponents)
        {
            if (emitter->GetPlaybackID() == id) emitter->OnPlaybackFinished();
        }
    }
    
    m_finishedPlaybacks.clear();
}

bool Droplet::Audio::AudioSystem::DetachEffect(dalia::EffectHandle &p_effect)
{
    dalia::Result res;
    res = m_daliaEngine->DetachEffect(p_effect);
    if (res != dalia::Result::Ok)
    {
        return false;
    }
    return true;
}

bool Droplet::Audio::AudioSystem::DestroyEffect(dalia::EffectHandle &p_effect)
{
    dalia::Result res;
    res = m_daliaEngine->DestroyEffect(p_effect);
    if (res != dalia::Result::Ok)
    {
        std::cout << "Could not destroy effect" << std::endl;
        return false;
    }
    return true;
}

bool Droplet::Audio::AudioSystem::ChangeEffectSetting(dalia::EffectHandle &p_effect, dalia::BiquadParams &p_params)
{
    dalia::Result res;
    res = m_daliaEngine->SetEffectParams(p_effect, p_params);
    if (res != dalia::Result::Ok)
    {
        return false;
    }
    return true;
}

void Droplet::Audio::AudioSystem::RegisterEffects()
{
    //TODO Every effect type that we create in droplet gets registered here to be used later
    //EffectRegistry::RegisterEffect("ReverbEffect", []() {return std::make_unique<ReverbEffect>();});
    EffectRegistry::RegisterEffect("LowPassEffect", []() {return std::make_unique<LowPassEffect>();});
    EffectRegistry::RegisterEffect("HighPassEffect", []() {return std::make_unique<HighPassEffect>();});
}
