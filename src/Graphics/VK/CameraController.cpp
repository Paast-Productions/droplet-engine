#include "CameraController.hpp"

#include <GameInput.hpp>

using namespace Droplet::Graphics;

void CameraController::UpdateCamera(Camera &p_camera, float p_deltaTime)
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
}