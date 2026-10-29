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
		static ViewOverlayUI &Get()
		{
			static ViewOverlayUI instance;
			return instance;
		}

		static void QueueOverlay(const std::function<void()> &p_overlayFunc)
		{
			Get().m_overlayQueue.push_back(p_overlayFunc);
		}

		const std::vector<std::function<void()>> &GetQueue() const
		{
			return m_overlayQueue;
		}

		void ClearQueue()
		{
			m_overlayQueue.clear();
		}

	private:
		ViewOverlayUI() = default;

		std::vector<std::function<void()>> m_overlayQueue;
	};
}