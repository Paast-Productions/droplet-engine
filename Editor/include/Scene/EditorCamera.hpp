  #pragma once
#include <vector>
#include <memory>
#include <ImGui/imgui.h>
#include <EditorCamera.hpp>
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

		[[nodiscard]] std::uint32_t GetTargetIndex() const { return m_targetIndex; }
		void SetTargetIndex(std::uint32_t p_renderTarget) { m_targetIndex = p_renderTarget; }

		[[nodiscard]] std::uint32_t GetTextureIndex() const { return m_textureIndex; }
		void SetTextureIndex(std::uint32_t p_textureIndex) { m_textureIndex = p_textureIndex; }

	private:

		/// @brief The index of the texture that the renderer will render to
		std::uint32_t m_targetIndex{ Graphics::VK::INVALID_INDEX };

		/// @brief The index of the texture that ImGui uses to render the scene in a ImGui window
		std::uint32_t m_textureIndex{ Graphics::VK::INVALID_INDEX };
	};
}