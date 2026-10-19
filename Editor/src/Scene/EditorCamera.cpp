#include "EditorCamera.hpp"
#include <EditorConfig.hpp>

using namespace Droplet::Editor::Scene;

EditorCamera::EditorCamera()
{
	// Reinstate the camera's position, orientation, and other properties
	glm::vec3 position = EditorConfig::LoadVec3("Editor.Camera.Pos", glm::vec3(0, 0, 0));
	float yaw = EditorConfig::LoadFloat("Editor.Camera.Yaw", 0.0f);
	float pitch = EditorConfig::LoadFloat("Editor.Camera.Pitch", 0.0f);

	Camera::SetPosition(position);
	Camera::SetOrientation(yaw, pitch);
}

EditorCamera::~EditorCamera()
{
	// Store the camera's position, orientation, and other properties
	glm::vec3 position = Camera::GetPosition();
	float yaw = Camera::GetYaw();
	float pitch = Camera::GetPitch();

	EditorConfig::StoreVec3("Editor.Camera.Pos", position);
	EditorConfig::StoreFloat("Editor.Camera.Yaw", yaw);
	EditorConfig::StoreFloat("Editor.Camera.Pitch", pitch);
}

void EditorCamera::Update([[maybe_unused]] bool p_hovered, [[maybe_unused]] ImVec2 p_mousePos)
{
	// TODO: Implement camera update logic

}