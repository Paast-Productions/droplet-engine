#include <Scene/EditorCameraController.hpp>
#include "Audio/AudioSystem.hpp"
#include <Editor/ViewOverlayUI.hpp>
#include <iostream>
#include <Time.hpp>

using namespace Droplet::Editor::Scene;

EditorCameraController::EditorCameraController(EditorCamera &p_camera) : m_editorCamera(p_camera) 
	{
		m_movementSpeed = EditorConfig::LoadFloat("EditorCamera.MovementSpeed", 5.0f);
}

void EditorCameraController::Update()
{
	GameInput &input = GameInput::Get();
	float velocity = Time::Get().GetDeltaTime() * m_movementSpeed;

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

		m_editorCamera.Rotate(
			-dx * m_mouseSensitivity,
			-dy * m_mouseSensitivity
		);
	}

	static float showMovementSpeedTimer = 0.0f;

	if (input.KeyHeld(Key::KeyLCTRL))
	{
		float scrollDelta = input.GetMouseScroll();

		if (scrollDelta != 0.0f)
		{
			m_movementSpeed *= 1.0f + (scrollDelta * 0.05f);
			m_movementSpeed = std::max(m_movementSpeed, 0.1f);
			showMovementSpeedTimer = 2.0f;

		}

		if (input.MousePressed(Mouse::MMB))
		{
			m_movementSpeed = 5.0f; // Reset to default speed
			showMovementSpeedTimer = 2.0f;
		}
	}

	if (showMovementSpeedTimer > 0.0f)
	{
		showMovementSpeedTimer -= Time::Get().GetDeltaTime();

		ViewOverlayUI::QueueOverlay([=]() {
			ImGui::SetNextWindowBgAlpha(0.5f);
			ImGui::BeginChild("Movement Speed", ImVec2(0,0), 
				ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysAutoResize, 
				ImGuiWindowFlags_NoResize
			);
			ImGui::SetWindowFontScale(1.5f);
			ImGui::Text("Speed: %.2f", m_movementSpeed);
			ImGui::EndChild();
		});
	}
    
    Audio::AudioSystem::GetInstance().UpdateEditorListener3DAttributes(
        m_editorCamera.GetPosition(),
        m_editorCamera.GetForward(),
        m_editorCamera.GetUp(),
        { 0.0f, 0.0f, 0.0f } // Set velocity to 0 for now
    );
}

void EditorCameraController::CameraLock(bool p_enabled, float p_centerX, float p_centerY)
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

	input.LockCursor(m_lookActive, p_centerX, p_centerY);
}

void EditorCameraController::SetMovementSpeed(float p_speed) 
{ 
	m_movementSpeed = p_speed; 
	EditorConfig::StoreFloat("EditorCamera.MovementSpeed", m_movementSpeed);
}
