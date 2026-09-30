#include <SDL3/SDL.h>

#include "Graphics/VK/Camera.hpp"
#include <algorithm>
#include <iostream>

void Camera::ProcessInput(float p_deltaTime)
{
	const bool *keyboardState = SDL_GetKeyboardState(nullptr);

	if (keyboardState[SDL_SCANCODE_W])
	{
		ProcessKeyboard(CameraMovement::FORWARD, p_deltaTime);
	}
	if (keyboardState[SDL_SCANCODE_S])
	{
		ProcessKeyboard(CameraMovement::BACKWARD, p_deltaTime);
	}
	if (keyboardState[SDL_SCANCODE_D])
	{
		ProcessKeyboard(CameraMovement::RIGHT, p_deltaTime);
	}
	if (keyboardState[SDL_SCANCODE_A])
	{
		ProcessKeyboard(CameraMovement::LEFT, p_deltaTime);
	}
	if (keyboardState[SDL_SCANCODE_SPACE])
	{
		ProcessKeyboard(CameraMovement::UP, p_deltaTime);
	}
	if (keyboardState[SDL_SCANCODE_LCTRL])
	{
		ProcessKeyboard(CameraMovement::DOWN, p_deltaTime);
	}
}

Camera::Camera(
	glm::vec3 p_position,
	glm::vec3 p_up,
	float p_yaw,
	float p_pitch)
	: position(p_position),
	up(p_up),
	worldUp(up),
	yaw(p_yaw),
	pitch(p_pitch)
{
	UpdateCameraVectors();
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
{
	float velocity = movementSpeed * deltaTime;

	std::cout
		<< "deltaTime: " << deltaTime
		<< " | speed: " << movementSpeed
		<< " | velocity: " << velocity
		<< " | pos before: "
		<< position.x << ", "
		<< position.y << ", "
		<< position.z << '\n';

	switch (direction)
	{
	case CameraMovement::FORWARD:
		position += front * velocity;
		break;
	case CameraMovement::BACKWARD:
		position -= front * velocity;
		break;
	case CameraMovement::LEFT:
		position -= right * velocity;
		break;
	case CameraMovement::RIGHT:
		position += right * velocity;
		break;
	case CameraMovement::UP:
		position += up * velocity;
		break;
	case CameraMovement::DOWN:
		position -= up * velocity;
		break;
	}
	std::cout
		<< "pos after: "
		<< position.x << ", "
		<< position.y << ", "
		<< position.z << '\n';
}

void Camera::ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch)
{
	xOffset *= mouseSensitivity;
	yOffset *= mouseSensitivity;

	yaw += xOffset;
	pitch += yOffset;

	if (constrainPitch)
	{
		pitch = std::clamp(pitch, -89.0f, 89.0f);
	}

	UpdateCameraVectors();
}

void Camera::UpdateCameraVectors()
{
	glm::vec3 newFront;
	newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	newFront.y = sin(glm::radians(pitch));
	newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(newFront);

	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));

}

glm::mat4 Camera::GetViewMatrix() const
{
	return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::GetProjectionMatrix(float p_aspectRatio, 
									  float p_nearPlane, 
									  float p_farPlane) const
{
	glm::mat4 projection = glm::perspective(
		glm::radians(zoom),
		p_aspectRatio,
		p_nearPlane,
		p_farPlane
	);

	//projection[1][1] *= -1.0f;

	return projection;
	/*return glm::perspective(
		glm::radians(zoom), 
		p_aspectRatio, 
		p_nearPlane, 
		p_farPlane);*/
}
