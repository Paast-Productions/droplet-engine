#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <math/bounds/Frustum.hpp>
#include <math/bounds/Ray.hpp>

namespace Droplet::Graphics
{
	/// @brief This class handles view and projection of the rendering
	class Camera
	{
	public:
		/// @brief Camera Constructor
		/// @param p_position Position of the Camera
		/// @param p_up Up-vector of the Camera
		/// @param p_yaw The yaw of the Camera
		/// @param p_pitch The pitch of the Camera
		Camera(
			glm::vec3 p_position = glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3 p_up = glm::vec3(0.0f, -1.0f, 0.0f),
			float p_yaw = -90.0f, float p_pitch = 0.0f,
			float p_fov = 45.0f, float p_aspect = 1.0f
		);

		/// @brief Apply movement inputs on the camera
		/// @param p_direction A vector of the direction of which to apply velocity
		/// @param p_deltaTime Frame rate based value to multply the cameras velocity
		void Move(glm::vec3 p_direction, float p_deltaTime);
		
		/// @brief Apply mouse rotation on the camera
		/// @param p_xOffset Horizontal rotation offset
		/// @param p_yOffset Vertical rotation offset
		/// @param p_constrainPitch Bool that determines if the camera can rotate upwards indefinetely
		void Rotate(float p_xOffset, float p_yOffset, bool p_constrainPitch = true);

		/// @brief Get the camera position
		/// @returns glm::vec3 camera position
		[[nodiscard]] glm::vec3 GetPosition() const;
		
		/// @brief Get the vector of the direction the camera is facing
		/// @returns glm::vec3 Vector of which the camera is facing 
		[[nodiscard]] glm::vec3 GetForward() const;
		
		/// @brief Get the value of the direction to the right of the camera
		/// @returns glm::vec3 Vector to the right of the camera
		[[nodiscard]] glm::vec3 GetRight() const;
		
		/// @brief Get the value of the direction above the camera
		/// @returns glm::vec3 Vector above the camera
		[[nodiscard]] glm::vec3 GetUp() const;

		/// @brief Get the frustum of the camera
		/// @returns A frustum of the camera
		[[nodiscard]] Math::Frustum GetFrustum() const; 

		/// @brief Return the view matrix
		/// @returns A view matrix
		[[nodiscard]] glm::mat4 GetViewMatrix() const;

		/// @brief Getter function for the projection matrix
		/// @returns A projection matrix
		[[nodiscard]] glm::mat4 GetProjectionMatrix() const;

		/// @brief Gets the horizontal field of view (FOV) of the camera in degrees.
		[[nodiscard]] float GetFOV() const { return m_fov; }

		/// @brief Gets the aspect ratio of the camera.
		[[nodiscard]] float GetAspect() const { return m_aspect; }

		/// @brief Gets the near and far clipping planes of the camera. 
		[[nodiscard]] glm::vec2 GetPlanes() const { return m_planes; }

		/// @brief Gets the yaw (horizontal rotation) of the camera in degrees.
		[[nodiscard]] float GetYaw() const { return m_yaw; }

		/// @brief Gets the pitch (vertical rotation) of the camera in degrees.
		[[nodiscard]] float GetPitch() const { return m_pitch; }

		/// @brief Sets the position of the camera in world space.
		void SetPosition(const glm::vec3 &p_position) { m_position = p_position; }

		/// @brief Sets the orientation of the camera using yaw and pitch angles in degrees.
		void SetOrientation(float p_yaw, float p_pitch) { m_yaw = p_yaw; m_pitch = p_pitch; UpdateCameraVectors(); }

		/// @brief Sets the horizontal field of view (FOV) of the camera in degrees.
		void SetFOV(float p_fov) { m_fov = p_fov; }

		/// @brief Sets the aspect ratio of the camera.
		void SetAspect(float p_aspect) { m_aspect = p_aspect; }

		/// @brief Sets the near and far clipping planes of the camera. 
		void SetPlanes(float p_near, float p_far) { m_planes = glm::vec2(p_near, p_far); }

		/// @brief Converts a 2D position in normalized device coordinates (NDC) to a 3D ray in world space.
		/// @param p_viewPos A 2D position in normalized device coordinates (NDC), where (0, 0) is the top-left corner and (1, 1) is the bottom-right corner of the viewport.
		/// @return A 3D ray in world space originating from the camera position and passing through the specified NDC position.
		[[nodiscard]] Math::Ray GetViewRay(const glm::vec2 &p_viewPos) const;

	private:
		glm::vec3 m_position {};
		glm::vec3 m_forward {};
		glm::vec3 m_up {};
		glm::vec3 m_right {};
		glm::vec3 m_worldUp {};
		glm::vec2 m_planes = glm::vec2(0.1f, 100.0f); // near and far planes

		float m_yaw {};
		float m_pitch {};
		float m_fov = 45.0f;		// horizontal field of view in degrees
		float m_aspect = 1.0f;		// width / height

		/// @brief Updates the cameras direction vectors
		void UpdateCameraVectors();
	};
}