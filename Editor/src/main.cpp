#include <Engine/Engine.hpp>
#include <cstdio>
#include "HierarchyWindow.hpp"
#include "NodeInspectorWindow.hpp"
#include "ScriptsWindow.hpp"
#include <Resource/ResourceBrowser.hpp>
#include <Scene/SceneViewWindow.hpp>
#include <tracy/public/tracy/Tracy.hpp>
#include <EditorConfig.hpp>
#include <ImGuizmo/ImGuizmo.h>
#include <ImGui/UIStyle.hpp>

using namespace Droplet::Editor;

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
	// TODO: Check if Tracy is enabled and if so, sleep for a few seconds to allow the profiler to connect before starting the engine
	ZoneScopedN("Editor"); // NOTE: Scoped Tracy calls should almost always be on the first line of their scope

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

	// Set the script path for where the engine will look for scripts.
	Droplet::Script::ScriptSystem::Get().SetScriptPath(ROOT_PATH "/src/Scripts");

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

	// Setup default ImGui ini file
	if (!EditorConfig::LoadBool("Editor.HasDefaultLayout", false))
	{
		ImGui::LoadIniSettingsFromDisk(ROOT_PATH "/Editor/Layouts/Default.ini");
		EditorConfig::StoreBool("Editor.HasDefaultLayout", true);
	}

	{
		std::string currStyle = EditorConfig::LoadString("Editor.Style", "Default");
		UIStyle::LoadStyle(currStyle);
	}

	engine->GetSceneManager().LoadScene("TestScene"); // HACK

	std::shared_ptr<InteractionState> interactionState = std::make_shared<InteractionState>();

	HierarchyWindow hierarchyWindow(engine, interactionState);
	NodeInspectorWindow nodeInspectorWindow(engine, interactionState);
	ScriptsWindow scriptsWindow(engine);
	Resource::ResourceBrowser resourceBrowser(engine);
	Scene::SceneViewWindow sceneViewWindow(engine, interactionState);

	hierarchyWindow.Init();
	nodeInspectorWindow.Init();
	scriptsWindow.Init();
	resourceBrowser.Init();
	sceneViewWindow.Init();

	FrameMark;

	bool showStyleEditor = false;

	// Run main loop
	bool run = true;
	while (run)
	{
		ZoneScopedN("Main Loop"); // NOTE: Scoped Tracy calls should almost always be on the first line of their scope

		EditorConfig::Update();

		// ImGui
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

		// Main toolbar
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::BeginMenu("Layout"))
				{
					if (ImGui::MenuItem("Load Default Layout"))
					{
						ImGui::LoadIniSettingsFromDisk(ROOT_PATH "/Editor/Layouts/Default.ini");
					}

					if (ImGui::MenuItem("Save Default Layout"))
					{
						ImGui::SaveIniSettingsToDisk(ROOT_PATH "/Editor/Layouts/Default.ini");
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Style"))
				{
					if (ImGui::BeginMenu("Set Style"))
					{
						std::string currStyle = EditorConfig::LoadString("Editor.Style", "Default");
						
						std::vector<std::string> styles;
						UIStyle::GetStyleNames(styles);

						for (const auto &styleName : styles)
						{
							bool isSelected = (styleName == currStyle);
							if (ImGui::MenuItem(styleName.c_str(), nullptr, isSelected))
							{
								if (styleName != currStyle)
								{
									UIStyle::LoadStyle(styleName);
								}
							}
						}

						ImGui::EndMenu();
					}

					if (ImGui::MenuItem("Show Style Editor", nullptr, showStyleEditor))
					{
						showStyleEditor = !showStyleEditor;
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::BeginMenu("Manipulate"))
				{
					if (ImGui::MenuItem("World"))
					{
						EditorConfig::StoreInt("manipulateSpace", ImGuizmo::WORLD);
					}

					if (ImGui::MenuItem("Local"))
					{
						EditorConfig::StoreInt("manipulateSpace", ImGuizmo::LOCAL);
					}

					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		if (showStyleEditor)
		{
			ImGui::Begin("Style Editor", &showStyleEditor);

			if (ImGui::Button("Save Style"))
			{
				ImGui::OpenPopup("Save Style");
			}

			ImGui::SameLine();
			if (ImGui::Button("Reset Style"))
			{
				std::string currStyle = EditorConfig::LoadString("Editor.Style", "Default");

				UIStyle::LoadStyle(currStyle);
			}

			if (ImGui::BeginPopup("Save Style"))
			{
				static char styleName[128] = "";
				ImGui::InputText("Style Name", styleName, IM_ARRAYSIZE(styleName));

				if (ImGui::Button("Confirm"))
				{
					UIStyle::SaveStyle(styleName);
					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();
				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}

			ImGui::Separator();

			ImGui::ShowStyleEditor(nullptr);
			ImGui::End();
		}

		hierarchyWindow.Render();
		nodeInspectorWindow.Render();
		scriptsWindow.Render();
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
