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
			//std::cout << "Mouse is hovering in focused window" << std::endl;
			m_editorContext->GetEditorCameraController().Update(deltaTime, true);
			GameInput &input = GameInput::Get();

			if (input.MousePressed(Mouse::LMB)) {
				//m_editorContext->GetScene().Ray();
				// 
				// Hämta camerans properties såsom
				// position
				// front
				// för med dessa så kan vi skapa en ray genom att klicka på skärmen och få en riktning 
				// från skärm position till världs position och riktning
				// Skapa ray
				// 
				const EditorCamera &camera = m_editorContext->GetEditorCamera();
				glm::vec3 position = camera.GetPosition();//m_editorContext->GetEditorCamera().GetPosition();
				glm::vec3 front = camera.GetForward();//m_editorContext->GetEditorCamera().GetForward();

				float x = mousePosNormalizedToCamView.x * 2.0f - 1.0f;
				float y = 1.0f - mousePosNormalizedToCamView.y * 2.0f;

				/*glm::vec4 rayClip(x, y, 1.0f, 1.0f);
				glm::mat4 inverseProjection = glm::inverse(camera.GetProjectionMatrix(camAspect));

				glm::vec4 rayView = inverseProjection * rayClip;
				glm::mat4 inverseView = glm::inverse(camera.GetViewMatrix());

				glm::vec4 rayWorld = inverseView * rayView;

				Droplet::Math::Ray ray(position, rayWorld);
				*/

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

				//std::cout << "Screen Position: " << x << ", " << y << std::endl;
				//std::cout << "Camera Position: " << position.x << ", " << position.y << ", " << position.z << std::endl;
				std::cout << "Camera Front: " << front.x << ", " << front.y << ", " << front.z << std::endl;
				//std::cout << "Ray direction: " << direction.x << ", " << direction.y << ", " << direction.z << std::endl;
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
