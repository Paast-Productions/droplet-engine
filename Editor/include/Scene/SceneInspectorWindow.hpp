#pragma once
#include <EditorWindow.hpp>

namespace Droplet::Editor
{
	class SceneInspectorWindow : public EditorWindow
	{
	public:
		SceneInspectorWindow(EditorContext *p_editorContext) : EditorWindow(p_editorContext) {}
		~SceneInspectorWindow() = default;
		
	protected:
		void InitImpl() override;
		void CloseImpl() override;
		void RenderImpl() override;
		void RenderToolbar() override;
		
	private:
		
	};
}
