#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


enum class CameraMovement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

class Camera
{
public:
	Camera(
		glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3 up = glm::vec3(0.0f, -1.0f, 0.0f),
		float yaw = -90.0f,
		float pitch = 0.0f
	);

	glm::mat4 GetViewMatrix() const;
	glm::mat4 GetProjectionMatrix(float p_aspectRatio, float p_nearPlane = 0.1f, float p_farPlane = 100.f) const;

	void ProcessInput(float p_deltaTime);
	void ProcessKeyboard(CameraMovement p_direction, float p_deltaTime);
	void ProcessMouseMovement(float p_xOffset, float p_yOffset, bool p_comstrainPitch = true);
	void ProcessMouseScroll(float p_yOffset);

	glm::vec3 GetPosition() const;
	glm::vec3 GetFront() const;
	float GetZoom() const;

private:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	float yaw;
	float pitch;

	float movementSpeed = 2.5f;
	float mouseSensitivity = 0.1f;
	float zoom = 45.0f;

	void UpdateCameraVectors();
};