#include <Scene/EditorCameraController.hpp>
#include <iostream>

using namespace Droplet::Editor::Scene;


void EditorCameraController::Update(float p_deltaTime, bool p_allowInput)
{
	if (!p_allowInput) 
	{
		return;

	}
	GameInput &input = GameInput::Get();
	float velocity = p_deltaTime * m_movementSpeed;

	if (input.KeyHeld(Key::KeyW)) 
	{
		m_editorCamera.Move(m_editorCamera.GetForward(), velocity);
	}
	if (input.KeyHeld(Key::KeyS)) 
	{
		m_editorCamera.Move(-m_editorCamera.GetForward(), velocity);
	}
	if (input.KeyHeld(Key::KeyD)) 
	{
		m_editorCamera.Move(m_editorCamera.GetRight(), velocity);
	}
	if (input.KeyHeld(Key::KeyA)) 
	{
		m_editorCamera.Move(-m_editorCamera.GetRight(), velocity);
	}
	if (input.KeyHeld(Key::KeySpace)) 
	{
		m_editorCamera.Move(m_editorCamera.GetUp(), velocity);
	}
	if (input.KeyHeld(Key::KeyShift)) 
	{
		m_editorCamera.Move(-m_editorCamera.GetUp(), velocity);
	}

	if (m_relativeMouse)
	{
		m_editorCamera.Rotate(
					input.GetDeltaMouseX(),
					input.GetDeltaMouseY()
				);
				// Get window ptr and size to reset mouse to center
	}

	//if (input.MousePressed(Mouse::LMB))
	//{
	//	float deltaX = input.GetDeltaMouseX();
	//	float deltaY = input.GetDeltaMouseY();

	//	m_editorCamera.Rotate(deltaX, deltaY);

	//}
}
