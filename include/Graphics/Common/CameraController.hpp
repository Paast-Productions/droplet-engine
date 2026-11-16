#pragma once

<<<<<<<< HEAD:include/Graphics/Vulkan/CameraController.hpp
========
#include "Graphics/Common/Camera.hpp"

>>>>>>>> 68f5a1fc (Cleaning up and organising the Engine project for migration.):include/Graphics/Common/CameraController.hpp
namespace Droplet::Graphics
{
	/// @brief Class that controls the camera 
	class CameraController
	{ 
	public:
		//TODO: Make it able to update the mouse depending on SDL_Window and/or ImGui window, see EditorCameraController.hpp
		/// @brief Processes inputs made from the user
		/// @param p_camera The camera object that the uppdates will apply on
		/// @param p_deltaTime Frame rate based value to multiply the cameras velocity
		void UpdateCamera(Camera &p_camera, float p_deltaTime) const;
	
	private:
		float m_movementSpeed = 2.5f;
		float m_mouseSensitivity = 0.05f;

		bool m_relativeMouse = false;
	};
}