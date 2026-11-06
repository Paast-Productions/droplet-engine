#pragma once

#include "EditorWindow.hpp"
#include "EditorContext.hpp"

namespace Droplet::Editor
{
	class BusInspectorWindow : public EditorWindow
	{
	public:
		BusInspectorWindow(EditorContext *p_editorContext) : EditorWindow(p_editorContext) {};
		~BusInspectorWindow() = default;
		
		void SetCurrentBus(Audio::AudioBus *p_bus);

	protected:

		void InitImpl() override;
		void CloseImpl() override;
		void RenderImpl() override;

	private:
		

		Audio::AudioBus *m_currentBus = nullptr;
	};
}