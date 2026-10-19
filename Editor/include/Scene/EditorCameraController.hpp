#pragma once
#include <EditorCamera.hpp>
#include <GameInput.hpp>

namespace Droplet::Editor::Scene
{
	/// @brief A class that controls the camera movement in the editor
	class EditorCameraController
	{
	public:
		/// @brief Constructor with a reference to a camera that this class will control
		/// @param p_camera The EditorCamera that this class will control
		EditorCameraController(EditorCamera &p_camera) : m_editorCamera(p_camera){}

		/// @brief Update functions that register movement of the editorCamera
		/// @param p_deltaTime Time variable that ensures consinstent movement between different frames per second
		/// @param p_window The SDL_window the camera cursor should apply its changes to
		/// @param p_centerX X-axis position in screen space which to reset the cursor position to after rotation
		/// @param p_centerY Y-axis position in screen space which to reset the cursor position to after rotation
		void Update(float p_deltaTime, SDL_Window* p_window, float p_centerX, float p_centerY);

		/// @brief Function that controls if the camera should follow the cursor
		/// @param p_active Boolean if true sets the camera to follow the cursor
		void CameraLock(bool p_active);

	private:
		EditorCamera &m_editorCamera;

		float m_movementSpeed = 2.5f;
		float m_mouseSensitivity = 0.1f;

		bool m_lookActive = false;
	};
}