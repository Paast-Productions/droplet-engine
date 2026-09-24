#pragma once
#include "resource/IResource.hpp"

#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include <vector>
#include <string>

namespace Droplet
{
    /// @brief Class for animation resources.
	class AnimationResource : public IResource
	{
	public:
		/// @brief Struct for animation step data for a single bone.
	    struct BoneKeyframe
	    {
	        std::string boneName{};
	        glm::vec3 pos{ 0.f, 0.f, 0.f };
	        glm::quat rot{ 0.f, 0.f, 0.f, 1.f };
	        glm::vec3 scale{ 1.f, 1.f, 1.f };
	    };
		
		/// @brief Struct for keyframe data for the animation.
		struct AnimKeyframe
		{
			float time = 0.0f;
			std::vector<BoneKeyframe> boneKeyframes{};
		};
	    
	    /// @brief Sets the animation name
	    /// @param p_name The animation name to be set
	    void SetName(const std::string &p_name);

		/// @brief Sets whether the animation should loop.
		/// @param p_isLooping True if the animation should loop, false otherwise.
		void SetIsLooping(bool p_isLooping);

		/// @brief Sets the keyframes for the animation resource.
		/// @param p_keyframes The keyframes for the animation resource.
		void SetKeyframes(const std::vector<AnimKeyframe> &p_keyframes);
	    
	    /// @brief Gets the name of the animation
	    /// @return The name of the animation
	    [[nodiscard]] std::string_view GetName() const;

		/// @brief Gets whether the animation should loop.
		/// @return True if the animation should loop, false otherwise.
		[[nodiscard]] bool IsLooping() const;

		/// @brief Gets the duration of the animation.
		/// @return The duration of the animation in seconds.
		[[nodiscard]] float GetDuration() const;

		/// @brief Gets the keyframes for the animation resource.
		/// @return A vector of keyframes for the animation resource.
		[[nodiscard]] const std::vector<AnimKeyframe> &GetKeyframes() const;

	private:

	    std::string                 m_name{};                // Name of the animation
		bool						m_isLooping = false;	// Whether the animation should loop
		float						m_duration = 0.0f;		// Duration of the animation in seconds
		std::vector<AnimKeyframe>	m_keyframes{};			// Keyframes for the animation resource
	};
}
