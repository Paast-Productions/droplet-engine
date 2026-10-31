#include "EditorCamera.hpp"
#include <EditorConfig.hpp>

using namespace Droplet::Editor::Scene;

EditorCamera::EditorCamera()
{
	// Reinstate the camera's position, orientation, and other properties
	glm::vec3 position = EditorConfig::LoadVec3("Editor.Camera.Pos", glm::vec3(0, 0, 0));
	glm::vec3 fwd = EditorConfig::LoadVec3("Editor.Camera.Fwd", glm::vec3(0, 0, -1));
	glm::vec3 up = glm::vec3(0, 1, 0); // Default up vector
	glm::vec3 right = glm::normalize(glm::cross(fwd, up));
	up = glm::normalize(glm::cross(right, fwd)); // Recalculate up vector to ensure orthogonality

	Camera::SetPosition(position);
	Camera::SetOrientation(fwd, up);
}

EditorCamera::~EditorCamera()
{
	// Store the camera's position, orientation, and other properties
	EditorConfig::StoreVec3("Editor.Camera.Pos", GetPosition());
	EditorConfig::StoreVec3("Editor.Camera.Fwd", GetForward());
}
