#include "CameraController.hpp"

#include <GameInput.hpp>

using namespace Droplet::Graphics;

void CameraController::UpdateCamera(Camera &p_camera, float p_deltaTime, SDL_Window *p_window)
{
	float velocity = m_movementSpeed * p_deltaTime;

	if (GameInput::Get().KeyHeld(Key::KeyW))
	{
		p_camera.Move(p_camera.GetForward(), velocity);
	}
	if (GameInput::Get().KeyHeld(Key::KeyS))
	{
		p_camera.Move(-p_camera.GetForward(), velocity);
	}
	if (GameInput::Get().KeyHeld(Key::KeyD))
	{
		p_camera.Move(p_camera.GetRight(), velocity);
	}
	if (GameInput::Get().KeyHeld(Key::KeyA))
	{
		p_camera.Move(-p_camera.GetRight(), velocity);
	}
	if (GameInput::Get().KeyHeld(Key::KeySpace))
	{
		p_camera.Move(p_camera.GetUp(), velocity);
	}
	if (GameInput::Get().KeyHeld(Key::KeyShift))
	{
		p_camera.Move(-p_camera.GetUp(), velocity);
	}
	if (GameInput::Get().KeyPressed(Key::KeyQ))
	{
		m_relativeMouse = !m_relativeMouse;
		SDL_SetWindowRelativeMouseMode(p_window, m_relativeMouse);
	}

	if (m_relativeMouse)
	{
		p_camera.Rotate(
			-GameInput::Get().GetDeltaMouseX() * m_mouseSensitivity,
			GameInput::Get().GetDeltaMouseY() * m_mouseSensitivity
		);
		
		// Get window ptr and size to reset mouse to center
		int width {0}, height {0};
		SDL_GetWindowSize(p_window, &width, &height);
		GameInput::Get().SetCursorPosition(p_window, static_cast<float>(width) / 2, static_cast<float>(height) / 2);
	}
}