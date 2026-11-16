#pragma once
#ifdef _EDITOR
#include <ImGui/imgui.h>
#include <vector>
#include <functional>
#include <Time.hpp>

namespace Droplet::Editor
{
	/// @brief Represents the overlay UI elements that can be drawn on top of the scene view.
	class ViewOverlayUI
	{
	public:
		/// @brief Gets the singleton instance of the ViewOverlayUI.
		static ViewOverlayUI &Get()
		{
			static ViewOverlayUI instance;
			return instance;
		}

		/// @brief Queues an overlay function to be drawn on top of the scene view.
		static inline void QueueOverlay(const std::function<void()> &p_overlayFunc, float p_duration = 0.0f)
		{
			Get().m_overlayQueue.emplace_back(p_duration, p_overlayFunc);
		}

		/// @brief Gets the queue of overlay functions to be drawn on top of the scene view.
		const inline std::vector<std::pair<float, std::function<void()>>> &GetQueue() const
		{
			return m_overlayQueue;
		}

		/// @brief Updates the queue of overlay functions.
		void inline UpdateQueue()
		{
			float dT = Time::Get().GetDeltaTime();

			auto &queue = Get().m_overlayQueue;
			for (size_t i = 0; i < queue.size(); i++)
			{
				queue[i].first -= dT;

				if (queue[i].first <= 0.0f)
				{
					queue.erase(queue.begin() + i);
					i--;
				}
			}
		}

		/// @brief Clears the queue of overlay functions.
		void inline ClearQueue()
		{
			m_overlayQueue.clear();
		}

	private:
		ViewOverlayUI() = default;

		std::vector<std::pair<float, std::function<void()>>> m_overlayQueue;
	};
}
#endif
