#pragma once
#include <algorithm>

#include "Camera.hpp"
#include <Graphics/SDL/Window.hpp>

/// @brief Class that controlls the camera 
class CameraController
{ 
public:
	/// @brief Processes inputs made from the user
	/// @param p_camera The camera object that the uppdates will apply on
	/// @param p_deltaTime Frame rate based value to multiply the cameras velocity
	/// @param p_window The window handle to apply window options
	void UpdateCamera(Camera &p_camera, float p_deltaTime, const Droplet::Graphics::SDL::Window &p_window);
private:
	float m_movementSpeed = 2.5f;
	float m_mouseSensitivity = 0.1f;

	bool m_relativeMouse = false;
};