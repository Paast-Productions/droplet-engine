#pragma once
#include <vector>
#include <memory>
#include <ImGui/imgui.h>
#include <InteractionState.hpp>
#include <Graphics/VK/Camera.hpp>
//#include "../../include/Graphics/VK/Camera.hpp"

namespace Droplet::Editor::Scene
{
	/// @brief Represents a camera used in the editor for viewing and navigating the scene.
	class EditorCamera : public Droplet::Graphics::Camera
	{
	public:
		EditorCamera(InteractionState &p_interactionState) : m_interactionState(p_interactionState) {}
		~EditorCamera() = default;

		/// @brief Updates the camera's state based on user input and other factors.
		/// @param hovered Indicates whether the mouse is currently hovering over the camera's view area. This can be used to determine if the camera should respond to input events.
		/// @param mousePos Mouse position normalized to the camera view rect, where (0, 0) is the top-left corner and (1, 1) is the bottom-right corner.
		void Update(bool p_hovered, ImVec2 p_mousePos);

		/// @brief Gets the horizontal field of view (FOV) of the camera in degrees.
		[[nodiscard]] float GetFOV() const { return m_fov; }

		/// @brief Gets the aspect ratio of the camera. The aspect ratio is defined as the width divided by the height of the camera's view.
		[[nodiscard]] float GetAspect() const { return m_aspect; }

	private:

		// TODO: position, rotation, etc.

		InteractionState &m_interactionState;

		float m_fov = 45.0f;		// horizontal field of view in degrees
		float m_aspect = 1.0f;		// width / height
	};
}