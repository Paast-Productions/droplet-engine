#pragma once

#include <chrono>

namespace Droplet
{
	/// @brief A universal singleton Time class
	class Time
	{
	public:
		/// @brief Constructs the Time class.
		Time();

		~Time() = default;

		/// @brief Retrieves the static instance of the Time.
		/// @return Reference to the Time instance.
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

		/// @brief Get the fixed delta time in seconds.
		/// @returns The fixed delta time.
		[[nodiscard]] float GetFixedDeltaTime() const;

		/// @brief Set the fixed delta time in seconds.
		/// @param p_fixedDeltaTime The new fixed delta time.
		void SetFixedDeltaTime(const float p_fixedDeltaTime);

	private:
		std::chrono::high_resolution_clock::time_point m_startTime{};
		std::chrono::high_resolution_clock::time_point m_lastTime{};
		float m_deltaTime = 0.0f;
		float m_fixedDeltaTime = 1.0f / 20.0f;
		float m_accumulator = 0.0f;

		/// @brief Get the current time.
		/// @returns The current time.
		[[nodiscard]] std::chrono::high_resolution_clock::time_point GetCurrentTime() const;

	};
}
