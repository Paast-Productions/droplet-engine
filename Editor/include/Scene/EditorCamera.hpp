#pragma once
#include <vector>
#include <memory>
#include <ImGui/imgui.h>
#include <Common/RenderDesc.hpp>
#include <Common/Camera.hpp>

namespace Droplet::Editor::Scene
{
	/// @brief Represents a camera used in the editor for viewing and navigating the scene.
	class EditorCamera : public Common::Camera
	{
	public:
		EditorCamera();
		~EditorCamera();

		[[nodiscard]] Common::RenderTargetDesc &GetTarget() { return m_target; }
		void SetTarget(const Common::RenderTargetDesc &p_target) { m_target = p_target; }

	private:

		/// @brief What the editor view renders into
		Common::RenderTargetDesc m_target {};
	};
}