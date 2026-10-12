#include "SceneViewWindow.hpp"

using namespace Droplet::Editor::Scene;

void SceneViewWindow::RenderImpl()
{
	ImDrawList *drawList = ImGui::GetWindowDrawList();

	// Calculate the camera view rect, so that inputs relative to the camera view rect can be collected and used during the camera update.

	ImVec2 contentRegion = ImGui::GetContentRegionAvail();
	ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
	ImVec2 windowPos = ImGui::GetWindowPos();

	float camAspect = m_editorCamera.GetAspect();
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
		m_editorCamera.Update(isMouseHovering, mousePosNormalizedToCamView);
	}

	// TODO: Request the engine renderer to render the camera view to a texture

	// TODO: Wait for the renderer to finish rendering the camera view to the texture
	ImTextureRef camViewTexture = nullptr; // TODO: Replace with the actual texture reference of the rendered camera view.
	// Funnily enough, rendering this nullptr renders ImGui's symbol atlas texture, which is a nice placeholder for now.
	
	ImVec2 camViewPos = { 
		camViewRectMin.x - windowPos.x, 
		camViewRectMin.y - windowPos.y 
	};

	// Color entire window background with a dark gray color
	drawList->AddRectFilled(
		cursorScreenPos - ImVec2(8, 8), 
		cursorScreenPos + contentRegion + ImVec2(8, 8), 
		IM_COL32(8, 8, 8, 255)
	);

	// Render the camera view texture to the scene view window
	ImGui::SetCursorPos(camViewPos);
	ImGui::Image(camViewTexture, camSize, ImVec2(0, 1), ImVec2(1, 0));

	// HACK: Render a placeholder rectangle for the camera view until the actual camera view texture is available.
	drawList->AddRect(camViewRectMin, camViewRectMax, IM_COL32(255, 0, 0, 128), 0.0f, 0, 4.0f);

	// Capture mouse interaction
	ImGui::SetCursorPos(camViewPos);
	if (ImGui::InvisibleButton("CameraView", camSize))
	{
		// TODO: Take focus
	}
}
