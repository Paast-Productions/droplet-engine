#include <Scene/EditorCameraController.hpp>
#include <iostream>
#include <Time.hpp>

using namespace Droplet::Editor::Scene;


void EditorCameraController::Update(SDL_Window* p_window, [[maybe_unused]] float p_centerX, [[maybe_unused]] float p_centerY)
{
	GameInput &input = GameInput::Get();
	float velocity = Time::Get().GetDeltaTime() * m_movementSpeed;

	if (input.KeyHeld(Key::W)) 
	{
		m_editorCamera.Move(m_editorCamera.GetForward(), velocity);
	}
	if (input.KeyHeld(Key::S)) 
	{
		m_editorCamera.Move(-m_editorCamera.GetForward(), velocity);
	}
	if (input.KeyHeld(Key::D)) 
	{
		m_editorCamera.Move(m_editorCamera.GetRight(), velocity);
	}
	if (input.KeyHeld(Key::A)) 
	{
		m_editorCamera.Move(-m_editorCamera.GetRight(), velocity);
	}
	if (input.KeyHeld(Key::Space)) 
	{
		m_editorCamera.Move(m_editorCamera.GetUp(), velocity);
	}
	if (input.KeyHeld(Key::LShift)) 
	{
		m_editorCamera.Move(-m_editorCamera.GetUp(), velocity);
	}

	if (m_lookActive)
	{
		float dx = input.GetDeltaMouseX();
		float dy = input.GetDeltaMouseY();

		m_editorCamera.Rotate(
			dx * m_mouseSensitivity,
			-dy * m_mouseSensitivity
		);

		input.SetCursorPosition(p_window, true, p_centerX, p_centerY);
		//input.MoveCursor(glm::vec2(p_centerX, p_centerY), true, true);
	}
}

void EditorCameraController::CameraLock(bool p_enabled)
{
	if (m_lookActive == p_enabled)
		return;

	m_lookActive = p_enabled;

	GameInput &input = GameInput::Get();
	ImGuiIO &io = ImGui::GetIO();

	if (p_enabled)
	{
		io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	}
	else
	{
		io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
	}

	input.HideCursor(m_lookActive);
}
