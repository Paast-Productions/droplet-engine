#pragma once

#include <chrono>

namespace Droplet
{
	/// @brief A universal Time class
	class Time
	{
	public:
		/// @brief Constructs the Time class.
		Time();

		/// @brief Constructs the Time class.
		/// @param p_fixedDeltaTime A parameter to set the fixed delta time.
		Time(const float p_fixedDeltaTime);
		~Time() = default;

		[[nodiscard]] static Time &Get()
		{
			static Time time;
			return time;
		}

		/// @brief Updates the delta time variable.
		/// @note This needs to be run in the beginning of the game loop.
		void Update();

		/// @brief Get the delta time in seconds.
		/// @returns The delta time.
		[[nodiscard]] float GetDeltaTime() const;

		/// @brief Get the runtime in seconds.
		/// @returns The runtime since start.
		[[nodiscard]] float GetRuntime() const;

		/// @brief Get the current time.
		/// @returns The current time.
		[[nodiscard]] std::chrono::high_resolution_clock::time_point GetCurrentTime() const;

		/// @brief Get the fixed delta time in seconds.
		/// @returns The fixed delta time.
		[[nodiscard]] float GetFixedDeltaTime() const;

		/// @brief Check if a fixed update should be executed.
		/// @returns True if fixed update should be executed, false if not.
		[[nodiscard]] bool ShouldFixedUpdate();

	private:
		std::chrono::high_resolution_clock::time_point m_startTime{};
		std::chrono::high_resolution_clock::time_point m_lastTime{};
		float m_deltaTime = 0.0f;
		float m_fixedDeltaTime = 1.0f / 20.0f;
		float m_accumulator = 0.0f;

	};
}
