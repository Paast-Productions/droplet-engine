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
		/// @param p_editorContext is a pointer that holds information about scene, camera, interactions and renderer
		SceneViewWindow(EditorContext *p_editorContext) :
			m_editorContext(p_editorContext) {}

		~SceneViewWindow() = default;

		// TODO: Possible <REFACTOR>
		/// @brief Get the ScenViewWindow rendertarget
		/// @returns custom RenderTarget class
		Droplet::Graphics::RenderTarget &GetRenderTarget();
		
		void SetViewDescriptor(VkDescriptorSet p_textureDescriptor);
		// TODO: Possible </REFACTOR> 

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

		EditorContext *m_editorContext { nullptr };

		/// @brief The texture that the renderer will render to
		Graphics::RenderTarget m_renderTarget { nullptr };

		/// @brief The texture that ImGui uses to render the scene in a ImGui window
		VkDescriptorSet m_textureDescriptorSet = VK_NULL_HANDLE;

		/// @brief Renders the ImGuizmo gizmo for manipulating objects in the scene view.
		/// @param p_sceneViewMin The minimum coordinates of the scene view rectangle.
		/// @param p_sceneViewMax The maximum coordinates of the scene view rectangle.
		void RenderImGuizmo(ImVec2 p_sceneViewMin, ImVec2 p_sceneViewMax);
	};
}