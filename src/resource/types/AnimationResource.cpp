#include "resource/types/AnimationResource.hpp"

#include <algorithm>

namespace Droplet
{
    void AnimationResource::SetName(const std::string &p_name)
    {
        m_name = p_name;
    }

    void AnimationResource::SetIsLooping(bool p_isLooping)
    {
        m_isLooping = p_isLooping;
    }

    void AnimationResource::SetKeyframes(const std::vector<AnimKeyframe> &p_keyframes)
    {
        m_keyframes = p_keyframes; 

        // Sort the keyframes by time to ensure they are in the correct order
        std::sort(m_keyframes.begin(), m_keyframes.end(), [](const AnimKeyframe &a, const AnimKeyframe &b) {
            return a.time < b.time;
        });

        // Set duration to the time of the last keyframe
        if (!m_keyframes.empty())
        {
            m_duration = m_keyframes.back().time;
        }
        else
        {
            m_duration = 0.0f;
        }
    }

    std::string_view AnimationResource::GetName() const
    {
        return m_name;
    }

    bool AnimationResource::IsLooping() const
    {
        return m_isLooping;
    }

    float AnimationResource::GetDuration() const
    {
        return m_duration;
    }

    const std::vector<AnimationResource::AnimKeyframe> & AnimationResource::GetKeyframes() const
    {
        return m_keyframes;
    }
}
