#pragma once
#include <vector>
#include <memory>
#include <ImGui/imgui.h>
#include <Graphics/VK/RenderTarget.hpp>
#include <Graphics/VK/ImGuiTexture.hpp>
#include <Graphics/VK/Camera.hpp>
#include <Graphics/GraphicsTypes.hpp>

namespace Droplet::Editor::Scene
{
	/// @brief Represents a camera used in the editor for viewing and navigating the scene.
	class EditorCamera : public Droplet::Graphics::Camera
	{
	public:
		EditorCamera();
		~EditorCamera();

		[[nodiscard]] Graphics::RenderTarget GetTarget() const { return m_target; }
		void SetTarget(Graphics::RenderTarget p_target) { m_target = p_target; }

		[[nodiscard]] Graphics::ImGuiTexture GetTexture() const { return m_texture; }
		void SetTexture(Graphics::ImGuiTexture p_texture) { m_texture = p_texture; }

	private:

		/// @brief The index of the texture that the renderer will render to
		Graphics::RenderTarget m_target {};

		/// @brief The index of the texture that ImGui uses to render the scene in a ImGui window
		Graphics::ImGuiTexture m_texture {};
	};
}