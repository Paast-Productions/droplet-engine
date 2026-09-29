#include "Time.hpp"

namespace Droplet
{
	Time::Time()
	{
		m_startTime = GetCurrentTime();
		m_lastTime = GetCurrentTime();
	}

	Time::Time(const float p_fixedDeltaTime = 1.0f / 20.0f)
	{
		m_startTime = GetCurrentTime();
		m_lastTime = GetCurrentTime();
		m_fixedDeltaTime = p_fixedDeltaTime;
	}

	void Time::Update()
	{
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

	std::chrono::high_resolution_clock::time_point Time::GetCurrentTime() const
	{
		return std::chrono::high_resolution_clock::now();
	}

	float Time::GetFixedDeltaTime() const
	{
		return m_fixedDeltaTime;
	}

	bool Time::ShouldFixedUpdate()
	{
		if (m_accumulator >= m_fixedDeltaTime)
		{
			m_accumulator -= m_fixedDeltaTime;
			return true;
		}

		return false;
	}
}