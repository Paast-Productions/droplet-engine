#include <Scene/EditorCameraController.hpp>
#include <iostream>

using namespace Droplet::Editor::Scene;


void EditorCameraController::Update(float p_deltaTime, bool p_allowInput, SDL_Window* p_window, [[maybe_unused]] float p_centerX, [[maybe_unused]]  float p_centerY)
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

	if (m_lookActive)
	{
		float dx = input.GetDeltaMouseX();
		float dy = input.GetDeltaMouseY();

		//m_editorCamera.Rotate(
		//			-input.GetDeltaMouseX() * m_mouseSensitivity,
		//			input.GetDeltaMouseY() * m_mouseSensitivity
		//		);

		m_editorCamera.Rotate(
			-dx * m_mouseSensitivity,
			dy * m_mouseSensitivity
		);
		input.SetCursorPosition(p_window, true, p_centerX, p_centerY);
	}

	//if (input.MousePressed(Mouse::LMB))
	//{
	//	float deltaX = input.GetDeltaMouseX();
	//	float deltaY = input.GetDeltaMouseY();

	//	m_editorCamera.Rotate(deltaX, deltaY);

	//}
}

void EditorCameraController::CameraLock(bool p_enabled)
{
	if (m_lookActive == p_enabled)
		return;

	m_lookActive = p_enabled;


	ImGuiIO &io = ImGui::GetIO();

	if (p_enabled)
		io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	else
		io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;

	GameInput::Get().SetRelativeMouse(m_lookActive);
}
