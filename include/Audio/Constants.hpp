#pragma once

namespace Droplet::Audio
{
    using PlaybackID = uint32_t;
    constexpr PlaybackID C_INVALID_PLAYBACK_ID = 0;
    
    using ListenerID = uint32_t;
    constexpr ListenerID C_INVALID_LISTENER_ID = UINT32_MAX;
    constexpr ListenerID C_EDITOR_LISTENER_ID = 0;
    
    constexpr float C_VOLUME_DECIBEL_DEFAULT = 0.0f;
    constexpr float C_VOLUME_DECIBEL_MIN = -60.0f;
    constexpr float C_VOLUME_DECIBEL_MAX = 12.0f;
    
    constexpr float C_STEREO_PAN_LEFT = -1.0f;
    constexpr float C_STEREO_PAN_MIDDLE = 0.0f;
    constexpr float C_STEREO_PAN_RIGHT = 1.0f;
    
    constexpr float C_PITCH_DEFAULT = 1.0f;
    constexpr float C_PITCH_MIN = 0.1f;
    constexpr float C_PITCH_MAX = 4.0f;
	
    constexpr float C_MIN_DISTANCE_DEFAULT = 1.0f;
    constexpr float C_MIN_DISTANCE_MIN = 0.1f;
	
    constexpr float C_MAX_DISTANCE_DEFAULT = 100.0f;
    constexpr float C_MAX_DISTANCE_MAX = 10000.0f;
    
    constexpr uint32_t C_LISTENERS_MAX = 4;
	constexpr std::size_t MAX_EFFECTS = 4; // Dalia limits the amount of effects on each bus to 4
}