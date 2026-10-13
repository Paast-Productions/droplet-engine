#include "SceneViewWindow.hpp"
#include <chrono>
#include <iostream>

using namespace Droplet::Editor::Scene;

void SceneViewWindow::InitImpl()
{
	// TODO: Make rendertarget reziable and match ImGui window
	m_renderTarget = m_editorContext->GetRenderer().CreateRenderTarget(720, 720);
	m_textureDescriptor = m_editorContext->GetRenderer().RegisterImGuiTexture(*m_renderTarget);
}

void SceneViewWindow::RenderImpl()
{
	ImGui::Text("SCENE VIEW TEST");
	// Temporarily time variable to create deltaTime for our update
	typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;

	static TimePoint s_lastFrameTime{ std::chrono::high_resolution_clock::now() };
	const TimePoint  currentTime{ std::chrono::high_resolution_clock::now() };
	const float deltaTime = { std::chrono::duration<float>(currentTime - s_lastFrameTime).count() };
	s_lastFrameTime = currentTime;


	// Calculate the camera view rect, so that inputs relative to the camera view rect can be collected and used during the camera update.

	ImVec2 contentRegion = ImGui::GetContentRegionAvail();
	ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();

	float camAspect = m_editorContext->GetEditorCamera().GetAspect();
	float contentAspect = contentRegion.x / contentRegion.y;

	ImVec2 camSize{};
	ImVec2 camViewRectMin{};
	ImVec2 camViewRectMax{};

	if (camAspect > contentAspect) // The camera is wider than the content region, so we need to fit the camera width to the content region width.
	{
		camSize.x = contentRegion.x;
		camSize.y = contentRegion.x / camAspect;

		// Center the camera view rect vertically within the content region.
		camViewRectMin.x = cursorScreenPos.x;
		camViewRectMin.y = cursorScreenPos.y + (contentRegion.y - camSize.y) * 0.5f;
		camViewRectMax.x = camViewRectMin.x + camSize.x;
		camViewRectMax.y = camViewRectMin.y + camSize.y;
	}
	else // The camera is taller than the content region, so we need to fit the camera height to the content region height.
	{
		camSize.x = contentRegion.y * camAspect;
		camSize.y = contentRegion.y;

		// Center the camera view rect horizontally within the content region.
		camViewRectMin.x = cursorScreenPos.x + (contentRegion.x - camSize.x) * 0.5f;
		camViewRectMin.y = cursorScreenPos.y;
		camViewRectMax.x = camViewRectMin.x + camSize.x;
		camViewRectMax.y = camViewRectMin.y + camSize.y;
	}

	// Collect mouse and keyboard inputs for the camera update, if the mouse is within the camera view rect.


	bool isWindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

	if (isWindowFocused)
	{
		[[maybe_unused]] bool isMouseHovering = ImGui::IsMouseHoveringRect(camViewRectMin, camViewRectMax);

		// Get the mouse position relative to the camera view rect.

		ImVec2 mousePos = ImGui::GetIO().MousePos; // TODO: Potential failure-point. Ensure this is in screen-space.

		ImVec2 mousePosNormalizedToCamView = { 
			(mousePos.x - camViewRectMin.x) / (camViewRectMax.x - camViewRectMin.x),
			(mousePos.y - camViewRectMin.y) / (camViewRectMax.y - camViewRectMin.y)
		};

		// TODO: Send data to camera update (mousePosNormalizedToCamView, isMouseHovering)
		//m_editorContext->GetEditorCamera().Update(isMouseHovering, mousePosNormalizedToCamView);


		if (isWindowFocused && isMouseHovering)
		{
			ImVec2 center = {
				(camViewRectMin.x + camViewRectMax.x) * 0.5f,
				(camViewRectMin.y + camViewRectMax.y) * 0.5f
			};

			GameInput &input = GameInput::Get();
			EditorCameraController &controller = m_editorContext->GetEditorCameraController();
			controller.Update(deltaTime, true, center.x, center.y);

			if (input.MousePressed(Mouse::LMB)) {
				
				const EditorCamera &camera = m_editorContext->GetEditorCamera();
				glm::vec3 position = camera.GetPosition();
				glm::vec3 front = camera.GetForward();

				float x = mousePosNormalizedToCamView.x * 2.0f - 1.0f;
				float y = 1.0f - mousePosNormalizedToCamView.y * 2.0f;
				glm::vec4 nearClip(x, y, 0.0f, 1.0f);
				glm::vec4 farClip(x, y, 1.0f, 1.0f);

				glm::mat4 inverseProjection = glm::inverse(camera.GetProjectionMatrix(camAspect));

				glm::vec4 nearView = inverseProjection * nearClip;
				glm::vec4 farView = inverseProjection * farClip;

				nearView /= nearView.w;
				farView /= farView.w;

				glm::mat4 inverseView = glm::inverse(camera.GetViewMatrix());
				glm::vec4 nearWorld = inverseView * nearView;
				glm::vec4 farWorld = inverseView * farView;

				glm::vec3 direction = glm::normalize(
					glm::vec3(farWorld) -
					glm::vec3(nearWorld)
				);


			}

			if (input.KeyPressed(Key::KeyQ))
			{
				m_cameraLookToggled = !m_cameraLookToggled;

				controller.CameraLock(m_cameraLookToggled);
			}

			//vk::Extent2D extent = m_renderTarget->GetExtent();
			
			if (input.MousePressed(Mouse::RMB))
			{
				std::cout << "CenterPosX: " << center.x << std::endl;
				std::cout << "CenterPosY: " << center.y << std::endl;
				std::cout << "MousePosX: " << mousePos.x << std::endl;
				std::cout << "MousePosY: " << mousePos.y << std::endl;
			}


		}

	}

	// TODO: Request the engine renderer to render the camera view to a texture

	// TODO: Wait for the renderer to finish rendering the camera view to the texture
	ImTextureRef camViewTexture = nullptr; // TODO: Replace with the actual texture reference of the rendered camera view.
	
	// Render the camera view texture to the scene view window
	ImGui::SetCursorPos({ camViewRectMin.x - cursorScreenPos.x, camViewRectMin.y - cursorScreenPos.y });
	ImGui::Image(ImTextureRef(m_textureDescriptor), camSize, ImVec2(0, 0), ImVec2(1, 1));
}

Droplet::Graphics::RenderTarget &SceneViewWindow::GetRenderTarget()
{
	return *m_renderTarget;
}