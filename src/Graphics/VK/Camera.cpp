#include <SDL3/SDL.h>
#include <algorithm>
#include <iostream>

#include "Graphics/VK/Camera.hpp"

#include "GameInput.hpp"


using namespace Droplet;
void Camera::ProcessInput(float p_deltaTime, const Droplet::Graphics::SDL::Window &p_window)
{
	//const bool *keyboardState = SDL_GetKeyboardState(nullptr);
	//if(keyboardState[SDL_SCANCODE_Q])
	if (GameInput::Get().KeyHeld(Key::KeyW))
	{
		ProcessKeyboard(CameraMovement::FORWARD, p_deltaTime);
	}
	if (GameInput::Get().KeyHeld(Key::KeyS))
	{
		ProcessKeyboard(CameraMovement::BACKWARD, p_deltaTime);
	}
	if (GameInput::Get().KeyHeld(Key::KeyD))
	{
		ProcessKeyboard(CameraMovement::RIGHT, p_deltaTime);
	}
	if (GameInput::Get().KeyHeld(Key::KeyA))
	{
		ProcessKeyboard(CameraMovement::LEFT, p_deltaTime);
	}
	if (GameInput::Get().KeyHeld(Key::KeySpace))
	{
		ProcessKeyboard(CameraMovement::UP, p_deltaTime);
	}
	if (GameInput::Get().KeyHeld(Key::KeyShift))
	{
		ProcessKeyboard(CameraMovement::DOWN, p_deltaTime);
	}
	if (GameInput::Get().KeyPressed(Key::KeyQ))
	{
		m_relativeMouse = !m_relativeMouse;
		SDL_SetWindowRelativeMouseMode(p_window.Get(), m_relativeMouse);
	}


	if (m_relativeMouse)
	{
		ProcessMouseMovement(
			-GameInput::Get().GetDeltaMouseX(),
			GameInput::Get().GetDeltaMouseY()
			);
		// Get window ptr and size to reset mouse to center
		SDL_Window *window = p_window.Get();
		int width, height;
		SDL_GetWindowSize(window, &width, &height);
		GameInput::Get().SetCursorPosition(window, static_cast<float>(width) / 2, static_cast<float>(height / 2));
	}
}

Camera::Camera(
	glm::vec3 p_position,
	glm::vec3 p_up,
	float p_yaw,
	float p_pitch)
	: m_position(p_position),
	m_up(p_up),
	m_worldUp(p_up),
	m_yaw(p_yaw),
	m_pitch(p_pitch)
{
	UpdateCameraVectors();
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
{
	float velocity = m_movementSpeed * deltaTime;

	switch (direction)
	{
	case CameraMovement::FORWARD:
		m_position += m_front * velocity;
		break;
	case CameraMovement::BACKWARD:
		m_position -= m_front * velocity;
		break;
	case CameraMovement::LEFT:
		m_position -= m_right * velocity;
		break;
	case CameraMovement::RIGHT:
		m_position += m_right * velocity;
		break;
	case CameraMovement::UP:
		m_position += m_up * velocity;
		break;
	case CameraMovement::DOWN:
		m_position -= m_up * velocity;
		break;
	}
}

void Camera::ProcessMouseMovement(float p_xOffset, float p_yOffset, bool p_constrainPitch)
{
	p_xOffset *= m_mouseSensitivity;
	p_yOffset *= m_mouseSensitivity;

	m_yaw += p_xOffset;
	m_pitch += p_yOffset;

	if (p_constrainPitch)
	{
		m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
	}

	UpdateCameraVectors();
}

void Camera::UpdateCameraVectors()
{
	glm::vec3 newFront;
	newFront.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	newFront.y = sin(glm::radians(m_pitch));
	newFront.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	m_front = glm::normalize(newFront);

	m_right = glm::normalize(glm::cross(m_front, m_worldUp));
	m_up = glm::normalize(glm::cross(m_right, m_front));

}

glm::mat4 Camera::GetViewMatrix() const
{
	return glm::lookAt(m_position, m_position + m_front, m_up);
}

glm::mat4 Camera::GetProjectionMatrix(float p_aspectRatio, 
									  float p_nearPlane, 
									  float p_farPlane) const
{
	glm::mat4 projection = glm::perspective(
		glm::radians(m_zoom),
		p_aspectRatio,
		p_nearPlane,
		p_farPlane
	);

	return projection;

}
