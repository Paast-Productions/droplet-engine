  #pragma once
#include <vector>
#include <memory>
#include <ImGui/imgui.h>
#include <EditorCamera.hpp>
#include <Graphics/VK/Camera.hpp>

namespace Droplet::Editor::Scene
{
	/// @brief Represents a camera used in the editor for viewing and navigating the scene.
	class EditorCamera : public Droplet::Graphics::Camera
	{
	public:
		EditorCamera();
		~EditorCamera();

	private:
	};
}