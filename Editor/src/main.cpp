#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_vulkan.h>
#include <ImGui/imgui_impl_sdl3.h>
#include <SDL3/SDL.h>
#include <cstdio>
#include <Engine/Engine.hpp>
#include "HierarchyWindow.hpp"
#include <NodeInspectorWindow.hpp>
#include <Graphics/VK/Renderer.hpp>
#include <GameInput.hpp>
#include <Resource/ResourceBrowser.hpp>
#include <Scene/SceneViewWindow.hpp>
#include <tracy/public/tracy/Tracy.hpp>
#include <EditorConfig.hpp>

using namespace Droplet::Editor;

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
	// TODO: Check if Tracy is enabled and if so, sleep for a few seconds to allow the profiler to connect before starting the engine
	ZoneScopedN("Editor"); // NOTE: Scoped Tracy calls should always be on the first line of the containing scope

	glm::vec2 windowSize = EditorConfig::LoadVec2("Editor.WindowSize", glm::vec2(1280, 720));

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	std::shared_ptr<Droplet::Engine> engine = std::make_shared<Droplet::Engine>(Droplet::EngineConfig (
		Droplet::Graphics::SDL::WindowConfig {
			static_cast<int>(windowSize.x), static_cast<int>(windowSize.y), {}
		},
		Droplet::UpdateFlags::EditorFlags
	));

	SDL_Window *wnd = engine->GetWindow();
	if (!wnd)
	{
		return 1;
	}

	// Setup SDL polling listener
	engine->AddEventListener([](SDL_Event &event) { // ImGui listener callback
		ImGui_ImplSDL3_ProcessEvent(&event); 
	});

	engine->AddEventListener([](SDL_Event &event) { // Window resize callback
		if (event.type == SDL_EVENT_WINDOW_RESIZED)
		{
			int width = event.window.data1;
			int height = event.window.data2;
			EditorConfig::StoreVec2("Editor.WindowSize", glm::vec2(width, height));
		}
	});

	std::shared_ptr<InteractionState> interactionState = std::make_shared<InteractionState>();

	HierarchyWindow hierarchyWindow(engine, interactionState);
	NodeInspectorWindow nodeInspectorWindow(interactionState);
	Resource::ResourceBrowser resourceBrowser;
	Scene::SceneViewWindow sceneViewWindow(interactionState);

	hierarchyWindow.Init();
	nodeInspectorWindow.Init();
	resourceBrowser.Init();
	sceneViewWindow.Init();

	FrameMark;

	// Run main loop
	bool run = true;
	while (run)
	{
		ZoneScopedN("Main Loop"); // NOTE: Scoped Tracy calls should always be on the first line of the containing scope

		// ImGui
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());


		hierarchyWindow.Render();
		nodeInspectorWindow.Render();
		resourceBrowser.Render();
		sceneViewWindow.Render();

		Droplet::DROPLET_RETURNTYPE returnType = engine->Update();

		switch (returnType)
		{
		case Droplet::DROPLET_RETURNTYPE::OK:
			break;

		case Droplet::DROPLET_RETURNTYPE::EXIT:
			run = false;
			break;

		default:
		case Droplet::DROPLET_RETURNTYPE::EXIT_ERROR:
			run = false;
			break;
		}

		//engine->Endframe
	}

	return 0;
}
