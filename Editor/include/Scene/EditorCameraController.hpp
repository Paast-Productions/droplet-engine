#pragma once
#include <EditorCamera.hpp>
#include <GameInput.hpp>
namespace Droplet::Editor::Scene
{
	class EditorCameraController
	{
	public:
		EditorCameraController(Droplet::Editor::Scene::EditorCamera &p_camera) : m_editorCamera(p_camera){}

		void Update(float p_deltaTime, bool p_allowInput);

	private:
		Droplet::Editor::Scene::EditorCamera &m_editorCamera;


		//std::unique_ptr<EditorContext> m_editorContext;

		float m_movementSpeed = 2.5f;
		float m_mouseSensitivity = 0.1f;

		bool m_relativeMouse = true;
	};
}