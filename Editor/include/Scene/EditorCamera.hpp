#pragma once
#include <vector>
#include <memory>
#include <ImGui/imgui.h>
#include <Graphics/Common/RenderDesc.hpp>
#include <Graphics/Common/Camera.hpp>
#include <Graphics/Common/GraphicsTypes.hpp>

namespace Droplet::Editor::Scene
{
	/// @brief Represents a camera used in the editor for viewing and navigating the scene.
	class EditorCamera : public Droplet::Graphics::Camera
	{
	public:
		EditorCamera();
		~EditorCamera();

		[[nodiscard]] const Graphics::Common::RenderDesc &GetTarget() const { return m_target; }
		void SetTarget(const Graphics::Common::RenderDesc &p_target) { m_target = p_target; }

	private:

		/// @brief What the editor view renders into
		Graphics::Common::RenderDesc m_target {};
	};
}