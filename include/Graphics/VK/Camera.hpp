#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Graphics/SDL/Window.hpp>


enum class CameraMovement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

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
	/// @brief Return the view matrix
	/// @returns A glm::mat4
	glm::mat4 GetViewMatrix() const;
	/// @brief Return the projection matrix
	/// @returns A glm::mat4
	/// @param p_aspectRatio
	/// @param p_nearPlane
	/// @param p_farPlane
	glm::mat4 GetProjectionMatrix(float p_aspectRatio, float p_nearPlane = 0.1f, float p_farPlane = 100.f) const;
	
	/// @brief Processes inputs made from the user
	/// @param p_deltaTime Frame rate based value to multiply the cameras velocity
	/// @param p_window The window handle to apply window options
	void ProcessInput(float p_deltaTime, const Droplet::Graphics::SDL::Window &p_window);
	/// @brief Apply movement inputs on the camera
	/// @param p_direction Enum of the direction of which to apply velocity
	/// @param p_deltaTime Frame rate based value to multply the cameras velocity
	void ProcessKeyboard(CameraMovement p_direction, float p_deltaTime);
	/// @brief Handles mouse movement
	/// @param p_xOffset Position on the x-axis of the screen
	/// @param p_yOffset Position on the y-axis of the screen
	/// @param p_constarainPitch Idk
	void ProcessMouseMovement(float p_xOffset, float p_yOffset, bool p_comstrainPitch = true);
	
	/// @brief Get the camera position
	/// @returns glm::vec3 camera position
	glm::vec3 GetPosition() const;
	/// @brief Get the vector of the direction the camera is facing
	/// @returns glm::vec3 Vector of which the camera is facing 
	glm::vec3 GetFront() const;
	/// @brief Get the value of the cameras zoom value
	/// @returns The value of how much the zoom is applied on the camera
	float GetZoom() const;

private:
	glm::vec3 m_position;
	glm::vec3 m_front;
	glm::vec3 m_up;
	glm::vec3 m_right;
	glm::vec3 m_worldUp;

	float m_yaw;
	float m_pitch;

	float m_movementSpeed = 2.5f;
	float m_mouseSensitivity = 0.1f;
	float m_zoom = 45.0f;

	bool m_relativeMouse = true;

	/// @brief Updates the cameras three vectors
	void UpdateCameraVectors();
};