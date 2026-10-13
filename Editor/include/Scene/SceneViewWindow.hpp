#pragma once
#include <vector>
#include <memory>
#include <Ray.hpp>
#include <glm/glm.hpp>
#include <EditorWindow.hpp>
#include <EditorCamera.hpp>
#include <EditorCameraController.hpp>
#include <InteractionState.hpp>
#include <EditorContext.hpp>
#include <Graphics/VK/RenderTarget.hpp>

namespace Droplet::Editor::Scene
{
	class SceneViewWindow : public EditorWindow
	{
	public:
		SceneViewWindow(EditorContext *p_editorContext) :  m_editorContext(p_editorContext){}
		~SceneViewWindow() = default;

		Droplet::Graphics::RenderTarget &GetRenderTarget();
	protected:

		void InitImpl() override;

		/// @brief Renders the editor camera view and handles user interactions within the scene view window.
		void RenderImpl() override;


	private:
		bool m_cameraLookToggled = false;

		EditorContext *m_editorContext = nullptr;

		std::unique_ptr<Droplet::Graphics::RenderTarget> m_renderTarget;
		VkDescriptorSet m_textureDescriptor = VK_NULL_HANDLE;
	};
}