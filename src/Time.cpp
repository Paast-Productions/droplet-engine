#include "Time.hpp"
#include <tracy/public/tracy/Tracy.hpp>

namespace Droplet
{
	Time::Time()
	{
		m_startTime = GetCurrentTime();
		m_lastTime = GetCurrentTime();
	}

	void Time::Update()
	{
		ZoneScoped;

		std::chrono::high_resolution_clock::time_point now = GetCurrentTime();

		m_deltaTime = std::chrono::duration<float>(now - m_lastTime).count();
		m_lastTime = now;

		m_accumulator += m_deltaTime;
	}

	float Time::GetDeltaTime() const
	{
		return m_deltaTime;
	}

	float Time::GetRuntime() const
	{
		return std::chrono::duration<float>(GetCurrentTime() - m_startTime).count();
	}

	float Time::GetFixedDeltaTime() const
	{
		return m_fixedDeltaTime;
	}

	void Time::SetFixedDeltaTime(const float p_fixedDeltaTime)
	{
		m_fixedDeltaTime = p_fixedDeltaTime;
	}

	std::chrono::high_resolution_clock::time_point Time::GetCurrentTime() const
	{
		return std::chrono::high_resolution_clock::now();
	}
}