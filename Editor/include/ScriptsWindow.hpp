#pragma once
#include <EditorWindow.hpp>
#include "InteractionState.hpp"

namespace Droplet::Editor
{
	class ScriptsWindow : public EditorWindow
	{
	public:
		ScriptsWindow(std::shared_ptr<InteractionState> p_interactionState);
		~ScriptsWindow() = default;

	protected:
		void InitImpl() override;
		void CloseImpl() override;
		void RenderImpl() override;
		void RenderToolbar() override;

	private:
		/// @brief Shared editor interaction state used to track node selection.
		std::shared_ptr<InteractionState> m_interactionState;
	};
}
