#pragma once
#include <vector>
#include <memory>
#include <ImGui/imgui.h>
#include <InteractionState.hpp>
#include <Graphics/VK/Camera.hpp>

namespace Droplet::Editor::Scene
{
	/// @brief Represents a camera used in the editor for viewing and navigating the scene.
	class EditorCamera : public Droplet::Graphics::Camera
	{
	public:
		EditorCamera();
		~EditorCamera();

		/// @brief Updates the camera's state based on user input and other factors.
		/// @param hovered Indicates whether the mouse is currently hovering over the camera's view area. This can be used to determine if the camera should respond to input events.
		/// @param mousePos Mouse position normalized to the camera view rect, where (0, 0) is the top-left corner and (1, 1) is the bottom-right corner.
		void Update(bool p_hovered, ImVec2 p_mousePos);

	private:
	};
}