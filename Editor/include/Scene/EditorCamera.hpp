#pragma once
#include <vector>
#include <memory>
#include <ImGui/imgui.h>
#include <Graphics/RenderDesc.hpp> // your Graphics::Common header
#include <Graphics/Vulkan/Camera.hpp>
#include <Graphics/GraphicsTypes.hpp>

namespace Droplet::Editor::Scene
{
	/// @brief Represents a camera used in the editor for viewing and navigating the scene.
	class EditorCamera : public Droplet::Graphics::Camera
	{
	public:
		EditorCamera();
		~EditorCamera();

		[[nodiscard]] const Graphics::Common::RenderTargetDesc &GetTarget() const { return m_target; }
		void SetTarget(const Graphics::Common::RenderTargetDesc &p_target) { m_target = p_target; }

	private:

		/// @brief What the editor view renders into
		Graphics::Common::RenderTargetDesc m_target {};
	};
}