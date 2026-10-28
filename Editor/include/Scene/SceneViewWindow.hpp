#pragma once
#include <vector>
#include <memory>
#include <Ray.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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

		/// @brief Gizmo matrix for a multi-node selection, kept between frames while the gizmo is in use
		glm::mat4 m_groupGizmoMatrix = glm::mat4(1.0f);
		/// @brief Aggregate selection bounds (min, max) relative to the group gizmo matrix
		glm::vec3 m_groupGizmoBounds[2] = { glm::vec3(-0.5f), glm::vec3(0.5f) };
		bool m_groupGizmoActive = false;
		
		/// @brief Pointer to a variable that is shared across several scenes
		EditorContext *m_editorContext = nullptr;

		/// @brief The index of the texture that the renderer will render to
		std::uint32_t m_targetIndex { Graphics::VK::INVALID_INDEX };

		/// @brief The index of the texture that ImGui uses to render the scene in a ImGui window
		std::uint32_t m_textureIndex { Graphics::VK::INVALID_INDEX };

		/// @brief Renders the ImGuizmo gizmo for manipulating objects in the scene view.
		/// @param p_sceneViewMin The minimum coordinates of the scene view rectangle.
		/// @param p_sceneViewMax The maximum coordinates of the scene view rectangle.
		void RenderImGuizmo(ImVec2 p_sceneViewMin, ImVec2 p_sceneViewMax);
		
		/// @brief Updates the SceneViews render target to match the cameraSize
		/// @param p_width The width of the render target
		/// @param p_height The height of the render target
		void UpdateRenderTarget(uint32_t p_width, uint32_t p_height);

		/// @brief Handles the editor camera controls that operate on the current selection:
		/// F focuses the selection, Alt + LMB drag orbits around it, MMB drag pans in the plane perpendicular to the
		/// camera's forward direction, and the scroll wheel moves the camera along its forward axis.
		/// Does nothing that depends on a selection when the selection is empty.
		/// @param p_viewWidth Width of the camera view in pixels, used to scale panning.
		void ProcessCameraControls(float p_viewWidth);

		void SetCameraPivot(const glm::vec3 &p_pivot, float p_radius);
		void ApplyCameraPivot();
		void DrawCameraPivot() const;

		/// @brief Camera navigation drag currently in progress
		enum class CameraDrag { None, Orbit, Pan };
		CameraDrag m_cameraDrag = CameraDrag::None;
		glm::vec3 m_cameraPivot = glm::vec3(0.0f);
		float m_cameraPivotRadius = 8.0f;
	};
}