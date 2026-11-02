#pragma once

#include <Graphics/VK/Camera.hpp>

namespace Droplet::Graphics
{
	/// @brief Class that controlls the camera 
	class CameraController
	{ 
	public:
		//TODO: Make it able to update the mouse depending on SDL_Window and/or ImGui window, see EditorCameraController.hpp
		/// @brief Processes inputs made from the user
		/// @param p_camera The camera object that the uppdates will apply on
		/// @param p_deltaTime Frame rate based value to multiply the cameras velocity
		void UpdateCamera(Camera &p_camera, float p_deltaTime);
	
	private:
		float m_movementSpeed = 2.5f;
		float m_mouseSensitivity = 0.05f;

		bool m_relativeMouse = false;
	};
}