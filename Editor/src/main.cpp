#include <Engine/Engine.hpp>
#include <cstdio>
#include "HierarchyWindow.hpp"
#include "NodeInspectorWindow.hpp"
#include "ScriptsWindow.hpp"
#include "BehaviourInspector.hpp"
#include <Resource/ResourceBrowser.hpp>
#include <Scene/SceneViewWindow.hpp>
#include <ScriptSystem/ScriptSystem.hpp>

#include <Graphics/VK/TestData.hpp>

#include <tracy/public/tracy/Tracy.hpp>

#include <GameInput.hpp>
#include <EditorContext.hpp>
#include <Scene/SceneViewWindow.hpp>
//#include <tracy/public/tracy/TracyVulkan.hpp>
//#include <tracy/public/tracy/TracyLua.hpp>
#include <EditorConfig.hpp>
#include <ImGuizmo/ImGuizmo.h>
#include <ImGui/UIStyle.hpp>
#include <core/FileExplorer.hpp>
#include <IconFontCppHeaders/IconsLucide.h>

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
		}
		//Droplet::UpdateFlags::EditorFlags
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
    
    // --- Font Setup ---
    float baseFontSize = 13.0f; // This should match the size of our default font
    
    ImGuiIO io = ImGui::GetIO();
    ImFontConfig baseConfig;
    baseConfig.SizePixels = baseFontSize;
    io.Fonts->AddFontDefault(&baseConfig);
    
    ImFontConfig iconsConfig;
    iconsConfig.MergeMode = true;
    iconsConfig.PixelSnapH = true;
    iconsConfig.GlyphOffset = ImVec2(0.0f, 3.0f);
    static const ImWchar iconsRanges[] = { ICON_MIN_LC, ICON_MAX_16_LC, 0 };
    io.Fonts->AddFontFromFileTTF(ROOT_PATH "/Editor/Fonts/lucide.ttf", baseFontSize, &iconsConfig, iconsRanges);
    // ------
    Droplet::Editor::EditorContext editorContext(engine);

	HierarchyWindow hierarchyWindow(engine, &editorContext);
	NodeInspectorWindow nodeInspectorWindow(engine, &editorContext);
	BehaviourInspector behaviourInspector(engine);
	ScriptsWindow scriptsWindow(engine);
	Resource::ResourceBrowser resourceBrowser(engine);
	Scene::SceneViewWindow sceneViewWindow(engine, &editorContext);

	hierarchyWindow.Init();
	nodeInspectorWindow.Init();
	behaviourInspector.Init();
	scriptsWindow.Init();
	resourceBrowser.Init();
	sceneViewWindow.Init();

	FrameMark;

	bool showStyleEditor = false;

	ImGui::SetNextWindowPos(ImVec2(1000.0f, 100.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(600.0f, 600.0f), ImGuiCond_Always);

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
		behaviourInspector.Render();
		scriptsWindow.Render(); 
		resourceBrowser.Render();
		sceneViewWindow.Render();

		Droplet::DROPLET_RETURNTYPE returnType = engine->Update(editorContext.GetEditorCamera(), sceneViewWindow.GetRenderTarget());

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
