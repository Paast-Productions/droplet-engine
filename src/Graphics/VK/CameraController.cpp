#include "CameraController.hpp"

#include <GameInput.hpp>

using namespace Droplet::Graphics;

void CameraController::UpdateCamera(Camera &p_camera, float p_deltaTime, const SDL::Window &p_window)
{
	float velocity = m_movementSpeed * p_deltaTime;

	if (GameInput::Get().KeyHeld(Key::KeyW))
	{
		p_camera.Move(p_camera.GetFront(), velocity);
	}
	if (GameInput::Get().KeyHeld(Key::KeyS))
	{
		p_camera.Move(-p_camera.GetFront(), velocity);
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
		SDL_SetWindowRelativeMouseMode(p_window.Get(), m_relativeMouse);
	}

	if (m_relativeMouse)
	{
		p_camera.Rotate(
			-GameInput::Get().GetDeltaMouseX() * m_mouseSensitivity,
			GameInput::Get().GetDeltaMouseY() * m_mouseSensitivity
		);
		// Get window ptr and size to reset mouse to center
		SDL_Window *window = p_window.Get();
		int width, height;
		SDL_GetWindowSize(window, &width, &height);
		GameInput::Get().SetCursorPosition(window, static_cast<float>(width) / 2, static_cast<float>(height / 2));
	}
}