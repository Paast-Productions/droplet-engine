#include "SceneViewWindow.hpp"
#include <math/bounds/Intersection.hpp>
#include <math/bounds/Conversion.hpp>
#include <SceneSystem/Scene.hpp>
#include <ImGuizmo/ImGuizmo.h>
#include <tracy/public/tracy/Tracy.hpp>
#include <EditorConfig.hpp>
#include <EditorShortcuts.hpp>
#include <Engine/EngineFlags.hpp>
#include <IconFontCppHeaders/IconsLucide.h>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <Graphics/DebugDraw.hpp>
#include <ImGuiUtils.hpp>

using namespace Droplet::Editor::Scene;
using namespace Droplet::Graphics::Debug;

namespace
{
	float GetFPSDisplayNum()
	{
		static std::vector<float> s_frameTimes;
		static float s_frameTimeCollectTimer = 0.0f;
		static float s_frameTimeCollectInterval = 0.2f;
		static float s_lastFPSAvg = 0.0f;

		float dT = ImGui::GetIO().DeltaTime;

		s_frameTimeCollectTimer += dT;
		s_frameTimes.push_back(dT);

		if (s_frameTimeCollectTimer >= s_frameTimeCollectInterval)
		{
			s_frameTimeCollectTimer -= s_frameTimeCollectInterval;

			float frameTimeSum = 0.0f;
			for (float frameTime : s_frameTimes)
			{
				frameTimeSum += frameTime;
			}

			s_lastFPSAvg = s_frameTimes.empty() ? 0.0f : ((float)s_frameTimes.size() / frameTimeSum);
			s_frameTimes.clear();
		}

		return s_lastFPSAvg;
	}

	using DNode = Droplet::Scene::Node;
	using DTransform = Droplet::Scene::Transform;

	/// @brief Gets the selected nodes that are valid for manipulation, excluding nodes whose ancestor is also selected
	/// (those follow their ancestor, and would otherwise be transformed twice).
	std::vector<std::shared_ptr<DNode>> GetManipulatableSelection(const std::vector<std::shared_ptr<DNode>> &p_selected)
	{
		std::vector<std::shared_ptr<DNode>> result;

		for (const auto &node : p_selected)
		{
			if (!node || !node->GetBounds())
			{
				continue;
			}

			bool ancestorSelected = false;
			for (auto parent = node->GetParent(); parent; parent = parent->GetParent())
			{
				if (std::find(p_selected.begin(), p_selected.end(), parent) != p_selected.end())
				{
					ancestorSelected = true;
					break;
				}
			}

			if (!ancestorSelected)
			{
				result.push_back(node);
			}
		}

		return result;
	}

	/// @brief Computes the world-space AABB (min, max) enclosing the world-space bounds of all given nodes.
	bool ComputeWorldBounds(const std::vector<std::shared_ptr<DNode>> &p_nodes, glm::vec3 &p_min, glm::vec3 &p_max)
	{
		bool any = false;

		for (const auto &node : p_nodes)
		{
			if (!node || !node->GetBounds())
			{
				continue;
			}

			Droplet::Math::OBB obb = node->GetBounds()->GetTransformedOBB(node->GetTransform().GetMatrix(DTransform::Space::World));
			glm::vec3 corners[8];
			obb.GetCorners(corners);

			for (const glm::vec3 &corner : corners)
			{
				p_min = any ? glm::min(p_min, corner) : corner;
				p_max = any ? glm::max(p_max, corner) : corner;
				any = true;
			}
		}

		return any;
	}
}

void SceneViewWindow::InitImpl()
{
	
	SetName("Scene View"); 
	ToggleToolbar(true);
	
	// TODO: 
	//     Move this to the renderer and camera class. 
	//     The camera should request the renderer to create a render target.
	m_renderTarget = m_editorContext->GetEngine().CreateRenderTarget(720, 720);
	m_textureDescriptorSet = m_editorContext->GetEngine().RegisterImGuiTexture(m_renderTarget);
}

void SceneViewWindow::RenderToolbar()
{
	if (ImGui::BeginMenu("Edit"))
	{
		if (ImGui::BeginMenu("Space"))
		{
			ImGuizmo::MODE currentMode = (ImGuizmo::MODE)EditorConfig::LoadInt("manipulateSpace", 0);

			if (ImGui::MenuItem("World", nullptr, currentMode == ImGuizmo::WORLD))
			{
				EditorConfig::StoreInt("manipulateSpace", ImGuizmo::WORLD);
			}

			if (ImGui::MenuItem("Local", nullptr, currentMode == ImGuizmo::LOCAL))
			{
				EditorConfig::StoreInt("manipulateSpace", ImGuizmo::LOCAL);
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Operation"))
		{
			ImGuizmo::OPERATION currentOperation = (ImGuizmo::OPERATION)EditorConfig::LoadInt("manipulateOperation");
			ImGuizmo::OPERATION translationOperation = ImGuizmo::TRANSLATE;
			ImGuizmo::OPERATION rotationOperation = ImGuizmo::ROTATE_X | ImGuizmo::ROTATE_Y | ImGuizmo::ROTATE_Z;
			ImGuizmo::OPERATION scaleOperation = ImGuizmo::SCALE;
			ImGuizmo::OPERATION boundsOperation = ImGuizmo::BOUNDS;
			ImGuizmo::OPERATION universalOperation = translationOperation | rotationOperation;

			if (ImGui::MenuItem("Universal", nullptr, currentOperation == universalOperation))
			{
				EditorConfig::StoreBool("manipulateBounds", true);
				EditorConfig::StoreInt("manipulateOperation", universalOperation);
			}

			if (ImGui::MenuItem("Translation", nullptr, currentOperation == translationOperation))
			{
				EditorConfig::StoreBool("manipulateBounds", false);
				EditorConfig::StoreInt("manipulateOperation", translationOperation);
			}

			if (ImGui::MenuItem("Rotation", nullptr, currentOperation == rotationOperation))
			{
				EditorConfig::StoreBool("manipulateBounds", false);
				EditorConfig::StoreInt("manipulateOperation", rotationOperation);
			}

			if (ImGui::MenuItem("Scale", nullptr, currentOperation == scaleOperation))
			{
				EditorConfig::StoreBool("manipulateBounds", false);
				EditorConfig::StoreInt("manipulateOperation", scaleOperation);
			}

			if (ImGui::MenuItem("Bounds", nullptr, currentOperation == boundsOperation))
			{
				EditorConfig::StoreBool("manipulateBounds", true);
				EditorConfig::StoreInt("manipulateOperation", boundsOperation);
			}

			ImGui::EndMenu();
		}

		float gizmoSize = EditorConfig::LoadFloat("GizmoSize", 0.1f);
		ImGui::SliderFloat("Gizmo size", &gizmoSize, 0.001, .3f);
		EditorConfig::StoreFloat("GizmoSize", gizmoSize);
		ImGuizmo::SetGizmoSizeClipSpace(gizmoSize);

		ImGui::EndMenu();
	}

	ImGui::Separator();

	float fpsDisplayNum = GetFPSDisplayNum();

	std::string fpsDisplayText = std::format("{:.0f}", fpsDisplayNum);
	ImGui::Text("FPS: %s", fpsDisplayText.c_str());
}

void SceneViewWindow::RenderImpl()
{
	ImDrawList *drawList = ImGui::GetWindowDrawList();
		
	ImVec2 contentRegion = ImGui::GetContentRegionAvail();
	ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
	ImVec2 windowPos = ImGui::GetWindowPos();
	
	GameInput &input = GameInput::Get();
	float contentAspect = contentRegion.x / contentRegion.y;
	vk::Extent2D extent = m_renderTarget.GetExtent();

	uint32_t width = static_cast<uint32_t>(contentRegion.x); 
	uint32_t height = static_cast<uint32_t>(contentRegion.y);
	// If mouse is released and viewSize is changed, Update the camera and renderTarget
	if (input.MouseReleased(Mouse::LMB))
	{	
		if (width != extent.width || height != extent.height)
		{
			if (contentRegion.x <= 0.0f || contentRegion.y <= 0.0f)
			{
				return;
			}

			if (input.MouseReleased(Mouse::LMB))
			{
				m_editorContext->GetEditorCamera().SetAspect(contentAspect);
				UpdateRenderTarget(width, height);
			}
		}
	}
	
	ImVec2 camSize = contentRegion;
	
	ImVec2 camViewRectMin = {
		cursorScreenPos.x + (contentRegion.x - camSize.x) * 0.5f,
		cursorScreenPos.y + (contentRegion.y - camSize.y) * 0.5f,
	};
	
	ImVec2 camViewRectMax = {
		camViewRectMin.x  + camSize.x,
		camViewRectMin.y + camSize.y
	};

	ImVec2 camViewPos = { 
		camViewRectMin.x - windowPos.x, 
		camViewRectMin.y - windowPos.y 
	};
	
	ImVec2 viewCenter = {
		(camViewRectMin.x + camViewRectMax.x) * 0.5f,
		(camViewRectMin.y + camViewRectMax.y) * 0.5f
	};

	if (m_cameraLookToggled)
	{
		ImGui::SetNextWindowFocus();
	}

	// Begin the child window that encompasses the camera view.
	ImGui::SetCursorPos(camViewPos);
	ImGui::BeginChild("CameraViewChild", camSize, ImGuiChildFlags_None, ImGuiWindowFlags_NoDecoration);

	// Process shortcut key presses
	if (ImGui::IsWindowFocused() && !m_cameraLookToggled)
	{
		EditorShortcuts::ProcessSceneViewKeyShortcuts();
	}

	ImTextureRef camViewTexture = m_textureDescriptorSet;

	// Render the camera view texture to the scene view window
	ImGui::Image(camViewTexture, camSize, ImVec2(0, 0), ImVec2(1, 1));

	m_hoveringSceneView = ImGui::IsItemHovered();

	bool isWindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

	ImVec2 imageMin = ImGui::GetItemRectMin();
	ImVec2 imageMax = ImGui::GetItemRectMax();

	const EditorCamera &camera = m_editorContext->GetEditorCamera();

	if (isWindowFocused)
	{
		bool isMouseHovering = ImGui::IsMouseHoveringRect(camViewRectMin, camViewRectMax);

		GameInput &input = GameInput::Get();
		EditorCameraController &controller = m_editorContext->GetEditorCameraController();

		// Get the mouse position relative to the camera view rect
		ImVec2 mousePos = ImGui::GetIO().MousePos;

		ImVec2 mousePosNormalizedToCamView = {
			(mousePos.x - camViewRectMin.x) / (camViewRectMax.x - camViewRectMin.x),
			(mousePos.y - camViewRectMin.y) / (camViewRectMax.y - camViewRectMin.y)
		};

		ImGuiViewport *viewport = ImGui::GetWindowViewport();

		SDL_Window *sceneWindow = nullptr;

		if (viewport)
		{
			SDL_WindowID windowID = static_cast<SDL_WindowID>(reinterpret_cast<uintptr_t>(viewport->PlatformHandle));
			sceneWindow = SDL_GetWindowFromID(windowID);
		}

		if (m_cameraLookToggled)
		{
			controller.Update(sceneWindow, viewCenter.x, viewCenter.y);
		}

	}

	if (m_hoveringSceneView || isWindowFocused)
	{
		if (input.KeyPressed(Key::KeyQ))
		{
			m_cameraLookToggled = !m_cameraLookToggled;
			m_editorContext->GetEditorCameraController().CameraLock(m_cameraLookToggled);

			ImGuiViewport *viewport = ImGui::GetWindowViewport();

			SDL_Window *sceneWindow = nullptr;

			if (viewport)
			{
				SDL_WindowID windowID = static_cast<SDL_WindowID>(reinterpret_cast<uintptr_t>(viewport->PlatformHandle));
				sceneWindow = SDL_GetWindowFromID(windowID);
			}

			input.SetCursorPosition(sceneWindow, false, viewCenter.x, viewCenter.y);
		}
	}

	// Click and rectangle (marquee) selection. A press starts a potential drag, and the selection is
	// resolved on release: a tiny drag is a click (raycast), a larger drag is a marquee selection.
	static bool isDragging = false;
	static ImVec2 dragStartPos;
	constexpr float c_minDragSize = 4.0f;

	// Shift or Ctrl makes the selection additive/toggling instead of replacing it
	const bool additiveSelection = input.KeyHeld(Key::KeyShift) || input.KeyHeld(Key::KeyLCTRL) || input.KeyHeld(Key::KeyRCTRL);

	if (!isDragging)
	{
		if (m_hoveringSceneView && !m_cameraLookToggled && !m_usingGizmo && !m_hoveringGizmo && !m_hoveringPlayButton
			&& input.MousePressed(Mouse::LMB))
		{
			isDragging = true;
			dragStartPos = ImGui::GetIO().MousePos;
		}
	}
	else if (m_cameraLookToggled || m_usingGizmo)
	{
		// Cancel the selection if the camera or gizmo took over the interaction
		isDragging = false;
	}
	else
	{
		const ImVec2 viewSize = { camViewRectMax.x - camViewRectMin.x, camViewRectMax.y - camViewRectMin.y };
		const ImVec2 mouseNow = ImGui::GetIO().MousePos;

		// Clamp the drag end to the view so the rectangle stays within the camera view
		ImVec2 dragEndPos = {
			std::clamp(mouseNow.x, camViewRectMin.x, camViewRectMax.x),
			std::clamp(mouseNow.y, camViewRectMin.y, camViewRectMax.y)
		};

		const float dragWidth = std::abs(dragEndPos.x - dragStartPos.x);
		const float dragHeight = std::abs(dragEndPos.y - dragStartPos.y);
		const bool isMarquee = dragWidth >= c_minDragSize && dragHeight >= c_minDragSize && viewSize.x > 0.0f && viewSize.y > 0.0f;

		glm::vec4 rectNormalized{
			(std::min(dragStartPos.x, dragEndPos.x) - camViewRectMin.x) / viewSize.x,
			(std::min(dragStartPos.y, dragEndPos.y) - camViewRectMin.y) / viewSize.y,
			(std::max(dragStartPos.x, dragEndPos.x) - camViewRectMin.x) / viewSize.x,
			(std::max(dragStartPos.y, dragEndPos.y) - camViewRectMin.y) / viewSize.y
		};

		Math::Frustum selectionFrustum{};

		if (isMarquee)
		{
			selectionFrustum = camera.ViewRectToWorldTile(rectNormalized);

			Math::Frustum drawFrustum = selectionFrustum;
			drawFrustum.GetNear().Slide(2.0f);

			DebugDraw::DrawFrustum(drawFrustum, glm::vec4(0.0f, 1.0f, 1.0f, 0.1f), false);
			DebugDraw::DrawFrustum(drawFrustum, glm::vec4(0.0f, 0.0f, 1.0f, 0.65f), false, true);
		}

		if (input.MouseReleased(Mouse::LMB) || !input.MouseHeld(Mouse::LMB))
		{
			isDragging = false;

			std::shared_ptr<Droplet::Scene::Scene> scene = m_editorContext->GetActiveScene();
			InteractionState &interactionState = m_editorContext->GetInteractionState();

			if (scene)
			{
				if (isMarquee)
				{
					// Only nodes fully inside the marquee are selected, found through the scene octree
					std::vector<std::shared_ptr<Droplet::Scene::Node>> hitNodes = scene->GetNodesFullyInFrustum(selectionFrustum);

					if (!additiveSelection)
					{
						interactionState.ClearNodeSelection();
					}

					for (const auto &node : hitNodes)
					{
						if (additiveSelection && input.KeyHeld(Key::KeyLCTRL) && interactionState.IsNodeSelected(node))
						{
							interactionState.DeselectNode(node);
						}
						else
						{
							interactionState.SelectNode(node, false);
						}
					}
				}
				else
				{
					// Treat as a click, as long as it was released over the camera view
					const bool releasedInView = mouseNow.x >= camViewRectMin.x && mouseNow.x <= camViewRectMax.x
						&& mouseNow.y >= camViewRectMin.y && mouseNow.y <= camViewRectMax.y;

					if (releasedInView && viewSize.x > 0.0f && viewSize.y > 0.0f)
					{
						glm::vec2 mousePosInView = glm::vec2(
							(mouseNow.x - camViewRectMin.x) / viewSize.x,
							(mouseNow.y - camViewRectMin.y) / viewSize.y
						);

						Math::RayHit hit{};
						auto hitNode = scene->RaycastNode(camera.GetViewRay(mousePosInView), &hit).lock();

						if (hit.didHit && hitNode)
						{
							if (additiveSelection)
							{
								// Toggle the hit node, keeping the rest of the selection
								if (interactionState.IsNodeSelected(hitNode))
								{
									interactionState.DeselectNode(hitNode);
								}
								else
								{
									interactionState.SelectNode(hitNode, false);
								}
							}
							else if (interactionState.IsNodeSelected(hitNode) && interactionState.GetSelectedNodes().size() == 1)
							{
								// Deselect the hit node if it is the only selected node
								interactionState.DeselectNode(hitNode);
							}
							else
							{
								interactionState.ClearNodeSelection();
								interactionState.SelectNode(hitNode);
							}
						}
						else if (!additiveSelection)
						{
							interactionState.ClearNodeSelection();
						}
					}
				}
			}
		}
	}
	
	RenderImGuizmo(camViewRectMin, camViewRectMax);

	// Render play/pause button
	{
		ImVec2 buttonSize = ImVec2(30, 30);
		ImVec2 buttonPos = ImVec2(
			camViewRectMin.x + camSize.x * 0.5f - buttonSize.x - 10,
			camViewRectMin.y + 10
		);

		bool isPlaying = EngineFlagsOwner::GetUpdateFlags() == Droplet::UpdateFlags::PlayModeFlags;

		ImGui::SetCursorScreenPos(buttonPos);
		if (isPlaying)
		{
			// Make red
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.0f, 0.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.3f, 0.3f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

			if (ImGui::Button(ICON_LC_PAUSE "##PauseButton", buttonSize))
			{
				EngineFlagsOwner::SetUpdateFlags(Droplet::UpdateFlags::EditorFlags);
			}

			ImGui::PopStyleColor(4);
		}
		else
		{
			// Make green
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.7f, 0.4f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

			if (ImGui::Button(ICON_LC_PLAY "##PlayButton", buttonSize))
			{	
				EngineFlagsOwner::SetUpdateFlags(Droplet::UpdateFlags::PlayModeFlags);
			}

			ImGui::PopStyleColor(4);
		}

		m_hoveringPlayButton = ImGui::IsItemHovered();
	}
	
	ImGui::EndChild();
}

void SceneViewWindow::RenderImGuizmo(ImVec2 p_sceneViewMin, ImVec2 p_sceneViewMax)
{
	ZoneScoped;
		
	using DScene = Droplet::Scene::Scene;

	// Setup
	ImVec2 rectPos = p_sceneViewMin;
	ImVec2 rectSize = p_sceneViewMax - p_sceneViewMin;

	ImGuizmo::SetRect(rectPos.x, rectPos.y, rectSize.x, rectSize.y);
	ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

	// Pre-fetch data
	InteractionState &interactionState = m_editorContext->GetInteractionState();

	EditorCamera &camera = m_editorContext->GetEditorCamera();
	glm::mat4 camViewMat = camera.GetViewMatrix();
	glm::mat4 camProjMat = camera.GetProjectionMatrix();

	std::shared_ptr<DScene> selectedScene = m_editorContext->GetActiveScene(); // TODO: Make weak pointer
	std::vector<std::shared_ptr<DNode>> nodes = selectedScene->GetRoots(); // TODO: Make weak pointer

	// DebugDraw wireframe around selected nodes by their bounds
	{
		auto selectedNodes = interactionState.GetSelectedNodes(); // TODO: Make weak pointer

		for (const auto &node : selectedNodes)
		{
			if (!node)
			{
				continue;
			}

			glm::mat4 nodeMat = node->GetTransform().GetMatrix(DTransform::Space::World);
			Math::OBB nodeOBB = node->GetBounds()->GetTransformedOBB(nodeMat);

			DebugDraw::DrawBox(
				nodeOBB,
				glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
				true, true
			);
		}
	}

	bool didUse = false;

	// Render transform gizmo for the selected nodes
	{
		// Nodes whose ancestor is also selected follow that ancestor, so they are excluded to avoid double transforms
		const auto &allSelected = interactionState.GetSelectedNodes();
		std::vector<std::shared_ptr<DNode>> manipulated = GetManipulatableSelection(allSelected);

		// TODO: Make this configurable by the user
		ImGuizmo::OPERATION currentOperation = static_cast<ImGuizmo::OPERATION>(EditorConfig::LoadInt("manipulateOperation", 0));
		ImGuizmo::MODE manipulateSpace = (ImGuizmo::MODE)EditorConfig::LoadInt("manipulateSpace", 0);
		bool useBounds = EditorConfig::LoadBool("manipulateBounds", false);

		if (manipulated.size() == 1)
		{
			const std::shared_ptr<DNode> &selectedNode = manipulated[0];

			Math::AABB nodeAABB = selectedNode->GetBounds()->GetAABB();
			glm::vec3 aabb[2] = { nodeAABB.GetMin(), nodeAABB.GetMax() };

			// If bounds should be active
			float *bounds = useBounds ? &(aabb[0].x) : nullptr;

			glm::mat4 nodeMatrix = selectedNode->GetTransform().GetMatrix(DTransform::Space::World);
			glm::mat4 deltaMatrix = glm::mat4(1.0f);

			bool result = ImGuizmo::Manipulate(
				&(camViewMat[0][0]),	// View matrix
				&(camProjMat[0][0]),	// Projection matrix
				currentOperation,		// Operation
				manipulateSpace,		// Space
				&(nodeMatrix[0][0]),	// Matrix to manipulate
				&(deltaMatrix[0][0]),	// Output delta matrix
				nullptr,				// Optional snap values
				bounds,					// Optional bounds
				nullptr					// Optional bounds snap values
			);

			if (result)
			{
				selectedNode->GetTransform().SetMatrix(nodeMatrix, DTransform::Space::World);
				didUse = true;
			}
		}
		else if (manipulated.size() > 1)
		{
			// The group gizmo is placed at the center of the aggregate world-space bounds of every selected node (including
			// selected descendants of selected nodes), axis-aligned.
			// While the gizmo is in use, the matrix is kept between frames so the pivot does not drift as nodes move.
			glm::vec3 boundsMin{}, boundsMax{};
			if (!m_groupGizmoActive || !ImGuizmo::IsUsing())
			{
				if (ComputeWorldBounds(allSelected, boundsMin, boundsMax))
				{
					m_groupGizmoMatrix = glm::translate(glm::mat4(1.0f), (boundsMin + boundsMax) * 0.5f);
					m_groupGizmoBounds[0] = boundsMin - (boundsMin + boundsMax) * 0.5f;
					m_groupGizmoBounds[1] = boundsMax - (boundsMin + boundsMax) * 0.5f;
					m_groupGizmoActive = true;
				}
				else
				{
					m_groupGizmoActive = false;
				}
			}

			if (m_groupGizmoActive)
			{
				// Draw the aggregate selection bounds
				glm::vec3 pivot = glm::vec3(m_groupGizmoMatrix[3]);
				if (!ImGuizmo::IsUsing())
				{
					DebugDraw::DrawBox(
						Math::OBB(pivot + (m_groupGizmoBounds[0] + m_groupGizmoBounds[1]) * 0.5f, (m_groupGizmoBounds[1] - m_groupGizmoBounds[0]) * 0.5f, glm::mat3(1.0f)),
						glm::vec4(1.0f, 0.8f, 0.2f, 1.0f),
						true, true
					);
				}

				float *bounds = useBounds ? &(m_groupGizmoBounds[0].x) : nullptr;

				glm::mat4 oldMatrix = m_groupGizmoMatrix;
				glm::mat4 deltaMatrix = glm::mat4(1.0f);

				bool result = ImGuizmo::Manipulate(
					&(camViewMat[0][0]),
					&(camProjMat[0][0]),
					currentOperation,
					manipulateSpace,
					&(m_groupGizmoMatrix[0][0]),
					&(deltaMatrix[0][0]),
					nullptr,
					bounds,
					nullptr
				);

				if (result)
				{
					// World-space delta of the group transform, applied around the group pivot to every node
					glm::mat4 groupDelta = m_groupGizmoMatrix * glm::inverse(oldMatrix);

					// Compute all new matrices before applying any, so no node is affected by another's update
					std::vector<glm::mat4> newMatrices;
					newMatrices.reserve(manipulated.size());
					for (const auto &node : manipulated)
					{
						newMatrices.push_back(groupDelta * node->GetTransform().GetMatrix(DTransform::Space::World));
					}

					for (size_t i = 0; i < manipulated.size(); i++)
					{
						// SetMatrix in world space accounts for the parent transform
						manipulated[i]->GetTransform().SetMatrix(newMatrices[i], DTransform::Space::World);
					}

					didUse = true;
				}
			}
		}
		else
		{
			m_groupGizmoActive = false;
		}
	}

	// Render view manipulation gizmo in the top-right corner of the scene view
	{
		static float viewGizmoLength = 8.0f;
		static float viewGizmoSize = 96.0f;
		static float viewGizmoMargin = 8.0f;
		static float drawPivotTimer = 0.0f;

		ImVec2 viewGizmoPos(p_sceneViewMax.x - viewGizmoSize - viewGizmoMargin, p_sceneViewMin.y + viewGizmoMargin);

		glm::mat4 editedView = camViewMat;
		ImGuizmo::ViewManipulate(
			&editedView[0][0], 
			viewGizmoLength, 
			viewGizmoPos, 
			ImVec2(viewGizmoSize, viewGizmoSize), 
			0x10101010
		);
		ImGui::LockToItem();

		if (ImGuizmo::IsUsingViewManipulate() && editedView != camViewMat)
		{
			// The view matrix is the inverse of the camera's world matrix, so the camera's basis and position are in its inverse
			glm::mat4 world = glm::inverse(editedView);
			glm::vec3 forward = glm::normalize(-glm::vec3(world[2]));

			float pitch = glm::degrees(glm::asin(glm::clamp(forward.y, -1.0f, 1.0f)));
			float yaw = glm::degrees(std::atan2(forward.z, forward.x));
			pitch = glm::clamp(pitch, -89.0f, 89.0f);

			camera.SetPosition(glm::vec3(world[3]));
			camera.SetOrientation(yaw, pitch);
		}

		if (ImGuizmo::IsViewManipulateHovered() || ImGuizmo::IsUsingViewManipulate())
		{
			// Draw the point around which the camera rotates when using the view manipulation gizmo.
			glm::vec3 pivot = camera.GetPosition() + camera.GetForward() * viewGizmoLength;
			Math::Sphere pivotSphere(pivot, 0.1f);
			DebugDraw::DrawSphere(pivotSphere, glm::vec4(1), 2, true);

			// Scroll to move the camera forward/backward while keeping the pivot point in the same place.
			float scrollDelta = -ImGui::GetIO().MouseWheel;
			if (scrollDelta != 0.0f)
			{
				float lengthChange = scrollDelta * 0.05f;
				viewGizmoLength *= 1.0f + lengthChange;
				viewGizmoLength = std::max(viewGizmoLength, 0.2f);

				camera.SetPosition(pivot - camera.GetForward() * viewGizmoLength);
			}
		}
	}

	m_usingGizmo = ImGuizmo::IsUsingAny() || ImGuizmo::IsUsingViewManipulate() || didUse;
	m_hoveringGizmo = ImGuizmo::IsOver() || ImGuizmo::IsOverBounds() || ImGuizmo::IsViewManipulateHovered();
}

Droplet::Graphics::RenderTarget &SceneViewWindow::GetRenderTarget()
{
	return m_renderTarget;
}

void SceneViewWindow::SetViewDescriptor(VkDescriptorSet p_textureDescriptor)
{
	m_textureDescriptorSet = p_textureDescriptor;
}

void SceneViewWindow::UpdateRenderTarget(uint32_t width, uint32_t height)
{
	auto &renderer = m_editorContext->GetEngine().GetRenderer();
	
	renderer.WaitIdle();
	
	if (m_textureDescriptorSet != VK_NULL_HANDLE)
	{
		ImGui_ImplVulkan_RemoveTexture(m_textureDescriptorSet);
		m_textureDescriptorSet = VK_NULL_HANDLE;
	}
	
	m_renderTarget = renderer.CreateRenderTarget(width, height);
	
	m_textureDescriptorSet = renderer.RegisterImGuiTexture(m_renderTarget);
}