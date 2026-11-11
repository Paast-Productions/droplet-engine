#pragma once
#include "resource/IResource.hpp"

#include <dalia/Sound.h>

namespace Droplet
{
    enum class SoundType : uint8_t
    {
        Resident = 0,
        Streaming = 1
    };
    
    class SoundResource : public IResource
    {
    public:
        SoundResource(dalia::SoundHandle p_handle);
        
        dalia::SoundHandle GetSoundHandle() const { return m_sound; }
        
    private:
        dalia::SoundHandle m_sound;
    };
}
