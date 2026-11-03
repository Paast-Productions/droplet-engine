#pragma once
#include <ImGui/imgui.h>
#include <vector>
#include <functional>

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
		static inline void QueueOverlay(const std::function<void()> &p_overlayFunc)
		{
			Get().m_overlayQueue.push_back(p_overlayFunc);
		}

		/// @brief Gets the queue of overlay functions to be drawn on top of the scene view.
		const inline std::vector<std::function<void()>> &GetQueue() const
		{
			return m_overlayQueue;
		}

		/// @brief Clears the queue of overlay functions.
		void inline ClearQueue()
		{
			m_overlayQueue.clear();
		}

	private:
		ViewOverlayUI() = default;

		std::vector<std::function<void()>> m_overlayQueue;
	};
}