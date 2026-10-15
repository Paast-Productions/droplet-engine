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
#include <Engine.hpp>

namespace Droplet::Editor::Scene
{
	/// @brief Represents the scene view window in the editor, which displays the camera view and handles editor camera interactions.
	class SceneViewWindow : public EditorWindow
	{
	public:
		/// @brief Constructor with a pointer to the editor context
		/// @param EditorContext is a pointer that holds information about scene, camera, interactions and renderer
		SceneViewWindow(std::shared_ptr<Droplet::Engine> p_engine, EditorContext *p_editorContext) : EditorWindow(p_engine), m_editorContext(p_editorContext){}
		~SceneViewWindow() = default;
		/// @brief Get the ScenViewWindow rendertarget
		/// @returns custom RenderTarget class
		Droplet::Graphics::RenderTarget &GetRenderTarget();
	protected:

		/// @brief Initializes the scene view window and sets its name.
		void InitImpl() override;

		void RenderToolbar() override;

		/// @brief Renders the editor camera view and handles user interactions within the scene view window.
		void RenderImpl() override;

	private:
		/// @brief Variable that controls to lock the camera or not
		bool m_cameraLookToggled = false;

		EditorContext *m_editorContext = nullptr;

		/// @brief The texture that the renderer will render to
		std::unique_ptr<Droplet::Graphics::RenderTarget> m_renderTarget;
		/// @brief The texture that ImGui uses to render the scene in a ImGui window
		VkDescriptorSet m_textureDescriptor = VK_NULL_HANDLE;
		void RenderImGuizmo(ImVec2 p_sceneViewMin, ImVec2 p_sceneViewMax);
	};
}