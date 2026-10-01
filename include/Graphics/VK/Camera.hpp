#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Graphics/SDL/Window.hpp>

/// @brief This class handles view and projection of the rendering
class Camera
{
public:
	Camera(
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3 up = glm::vec3(0.0f, -1.0f, 0.0f),
		float m_yaw = -90.0f,
		float m_pitch = 0.0f
	);

	/// @brief Apply movement inputs on the camera
	/// @param p_direction A vector of the direction of which to apply velocity
	/// @param p_deltaTime Frame rate based value to multply the cameras velocity
	void Move(glm::vec3 p_direction, float p_deltaTime);
	/// @brief Apply mouse rotation on the camera
	/// @param p_xOffset Horizontal rotation offset
	/// @param p_yOffset Vertical rotation offset
	/// @param p_constarainPitch Bool that determines if the camera can rotate upwards indefinetely
	void Rotate(float p_xOffset, float p_yOffset, bool p_constrainPitch = true);

	/// @brief Get the camera position
	/// @returns glm::vec3 camera position
	glm::vec3 GetPosition() const;
	/// @brief Get the vector of the direction the camera is facing
	/// @returns glm::vec3 Vector of which the camera is facing 
	glm::vec3 GetFront() const;
	/// @brief Get the value of the direction to the right of the camera
	/// @returns glm::vec3 Vector to the right of the camera
	glm::vec3 GetRight() const;
	/// @brief Get the value of the direction above the camera
	/// @returns glm::vec3 Vector above the camera
	glm::vec3 GetUp() const;

	/// @brief Return the view matrix
	/// @returns A glm::mat4
	glm::mat4 GetViewMatrix() const;

	/// @brief Return the projection matrix
	/// @returns A glm::mat4
	/// @param p_aspectRatio
	/// @param p_nearPlane
	/// @param p_farPlane
	glm::mat4 GetProjectionMatrix(float p_aspectRatio, float p_nearPlane = 0.1f, float p_farPlane = 100.f) const;

private:
	glm::vec3 m_position;
	glm::vec3 m_front;
	glm::vec3 m_up;
	glm::vec3 m_right;
	glm::vec3 m_worldUp;

	float m_yaw;
	float m_pitch;
	float m_zoom = 45.0f;

	/// @brief Updates the cameras three vectors
	void UpdateCameraVectors();
};