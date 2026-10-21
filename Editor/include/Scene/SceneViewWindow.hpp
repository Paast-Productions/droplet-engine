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
		SceneViewWindow(Droplet::Engine &p_engine, EditorContext *p_editorContext) : m_editorContext(p_editorContext){}

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
		bool m_hoveringGizmo = false;
		bool m_usingGizmo = false;
		bool m_hoveringSceneView = false;
		bool m_hoveringPlayButton = false;
		
		/// @brief Pointer to a variable that is shared across several scenes
		EditorContext *m_editorContext = nullptr;

		/// @brief The texture that the renderer will render to
		std::unique_ptr<Droplet::Graphics::RenderTarget> m_renderTarget;

		/// @brief The texture that ImGui uses to render the scene in a ImGui window
		VkDescriptorSet m_textureDescriptor = VK_NULL_HANDLE;

		/// @brief Renders the ImGuizmo gizmo for manipulating objects in the scene view.
		/// @param p_sceneViewMin The minimum coordinates of the scene view rectangle.
		/// @param p_sceneViewMax The maximum coordinates of the scene view rectangle.
		void RenderImGuizmo(ImVec2 p_sceneViewMin, ImVec2 p_sceneViewMax);
		
		/// @brief Updates the SceneViews render target to match the cameraSize
		/// @param p_width The width of the render target
		/// @param p_height The height of the render target
		void UpdateRenderTarget(uint32_t p_width, uint32_t p_height);
	};
}