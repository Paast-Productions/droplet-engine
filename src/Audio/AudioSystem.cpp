#include "Audio/AudioSystem.hpp"
#include "EffectRegistry.hpp"
#include "Effects/ReverbEffect.hpp"
#include "SceneSystem/Components/AudioListenerComponent.hpp"
#include "Resource/Types/SoundResource.hpp"
#include "SceneSystem/Components/AudioEmitterComponent.hpp"
#include <dalia/dalia.h>

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
	dalia::Result res = m_daliaEngine->Initialize(config);
	if (res != dalia::Result::Ok) return false;
    
    m_mixer.Initialize(*m_daliaEngine);
    
    RegisterEffects();
    
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
    
    // Sync players
    // for (auto* player : m_activePlayers)
    // {
    //     PlaybackID id = player->GetPlaybackID();
    //     auto it = m_activePlaybacks.find(id);
    //     if (it != m_activePlaybacks.end())
    //     {
    //         if (player->IsDirty())
    //         {
    //             m_daliaEngine->SetPlaybackVolumeDb(it->second, player->GetVolumeDb);
    //             player->ClearDirtyFlag();
    //         }
    //     }
    // }
    
    // Sync emitters
    for (auto* emitter : m_registeredEmitters)
    {
        PlaybackID id = emitter->GetPlaybackID();
        auto it = m_activePlaybacks.find(id);
        if (it != m_activePlaybacks.end())
        {
            dalia::PlaybackHandle daliaHandle = it->second;
            // Update spatial properties
            auto pos = emitter->GetWorldPosition();
            m_daliaEngine->SetPlaybackPosition(it->second, { pos.x, pos.y, pos.z });
            
            auto vel = emitter->GetVelocity();
            m_daliaEngine->SetPlaybackVelocity(it->second, { vel.x, vel.y, vel.z });
            
            if (emitter->IsDirty())
            {
                m_daliaEngine->SetPlaybackVolumeDb(daliaHandle, emitter->GetVolumeDb());
                m_daliaEngine->SetPlaybackRate(daliaHandle, emitter->GetPitch());
                m_daliaEngine->SetPlaybackLooping(daliaHandle, emitter->IsLooping());
                m_daliaEngine->SetPlaybackMinMaxDistance(daliaHandle, emitter->GetMinDistance(), emitter->GetMaxDistance());
                m_daliaEngine->SetPlaybackAttenuationCurve(daliaHandle, emitter->GetAttenuationCurve());
                if (!emitter->GetBusName().empty())
                {
                    m_daliaEngine->RoutePlayback(daliaHandle, emitter->GetBusName().c_str());
                }
                emitter->ClearDirtyFlag();
            }
        }
        
        // Sync listeners
        for (auto* listener : m_registeredListeners)
        {
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

void AudioSystem::RegisterEmitter(Scene::AudioEmitterComponent *emitter)
{
    m_registeredEmitters.push_back(emitter);
}

void AudioSystem::UnregisterEmitter(Scene::AudioEmitterComponent *emitter)
{
    auto it = std::find(m_registeredEmitters.begin(), m_registeredEmitters.end(), emitter);
    if (it != m_registeredEmitters.end()) m_registeredEmitters.erase(it);
}

void AudioSystem::RegisterListener(Scene::AudioListenerComponent *listener)
{
    for (uint32_t i = 1; i < 4; i++)
    {
        if (!m_listenerSlots[i])
        {
            m_listenerSlots[i] = true;
            listener->SetListenerId(i);
            m_registeredListeners.push_back(listener);
            m_daliaEngine->SetListenerActive(i, true);
            return;
        }
    }
    Debug::Log(Debug::LogType::Error, "AudioSystem: Maximum number of listeners (3) are already in use.");
}

void AudioSystem::UnregisterListener(Scene::AudioListenerComponent *listener)
{
    auto it = std::find(m_registeredListeners.begin(), m_registeredListeners.end(), listener);
    if (it != m_registeredListeners.end())
    {
        m_listenerSlots[listener->GetListenerId()] = false;
        m_registeredListeners.erase(it);
    }
}

PlaybackID AudioSystem::Play3D(Scene::AudioEmitterComponent *emitter, SoundResource *sound)
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
    m_daliaEngine->SetPlaybackRate(daliaHandle, emitter->GetPitch());
    m_daliaEngine->SetPlaybackLooping(daliaHandle, emitter->IsLooping());
    m_daliaEngine->SetPlaybackMinMaxDistance(daliaHandle, emitter->GetMinDistance(), emitter->GetMaxDistance());
    m_daliaEngine->SetPlaybackAttenuationCurve(daliaHandle, emitter->GetAttenuationCurve());
    
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

void AudioSystem::SetEditorListenerActive(bool p_active)
{
    m_daliaEngine->SetListenerActive(0, p_active);
    
    // Activate/deactivate listeners based if editor camera listener is active/inactive
    for (uint32_t i = 1; i < 4; i++)
    {
        if (m_listenerSlots[i] != false)
        {
            m_daliaEngine->SetListenerActive(i, !p_active);
        }
    }
}

void AudioSystem::UpdateEditorListener3DAttributes(const glm::vec3 &p_pos, const glm::vec3 &p_fwd,
    const glm::vec3 &p_up, const glm::vec3 &p_vel)
{
    m_daliaEngine->SetListener3DAttributes(C_EDITOR_LISTENER_INDEX, { 
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
        m_activePlaybacks.erase(id);
        
        // Notify players
        // if (auto* player : m_activePlayers) if (player->GetPlaybackID() == id) player->NotifyStopped();
        
        // Notify emitters
        for (auto* emitter : m_registeredEmitters)
        {
            if (emitter->GetPlaybackID() == id) emitter->OnPlaybackFinished();
        }
    }
    
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

void Droplet::Audio::AudioSystem::RegisterEffects()
{
    //TODO Every effect type that we create in droplet gets registered here to be used later
    EffectRegistry::RegisterEffect("ReverbEffect", []() {return std::make_unique<ReverbEffect>();});
}
