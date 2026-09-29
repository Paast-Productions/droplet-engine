#pragma once
#include <EditorWindow.hpp>
#include <EditorCamera.hpp>

namespace Droplet::Editor::Scene
{
	class SceneViewWindow : public EditorWindow
	{
	public:
		SceneViewWindow() = default;
		~SceneViewWindow() = default;

	protected:

		void RenderImpl() override;

	private:
		EditorCamera m_editorCamera;
	};
}