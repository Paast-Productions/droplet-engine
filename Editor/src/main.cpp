#include <Engine/Engine.hpp>
#include "HierarchyWindow.hpp"
#include "NodeInspectorWindow.hpp"
#include "SavedDataWindow.hpp"
#include "ScriptsWindow.hpp"
#include "BehaviourInspector.hpp"
#include <Resource/ResourceBrowser.hpp>
#include <Scene/SceneViewWindow.hpp>
#include <ScriptSystem/ScriptSystem.hpp>
#include <Graphics/VK/Camera.hpp>
#include <Graphics/VK/TestData.hpp>
#include <Graphics/DebugDraw.hpp>
#include <tracy/public/tracy/Tracy.hpp>
#include <GameInput.hpp>
#include <EditorContext.hpp>
#include <EditorConfig.hpp>
#include <ImGuizmo/ImGuizmo.h>
#include <ImGui/UIStyle.hpp>
#include <IconFontCppHeaders/IconsLucide.h>
#include <ImGui/imgui_stdlib.h>

using namespace Droplet::Editor;

const glm::vec2 C_INITSIZE = { 1280, 720 };

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
	// TODO: Check if Tracy is enabled and if so, sleep for a few seconds to allow the profiler to connect before starting the engine
	ZoneScopedN("Editor"); // NOTE: Scoped Tracy calls should almost always be on the first line of their scope

	glm::vec2 windowSize = EditorConfig::LoadVec2("Editor.WindowSize", C_INITSIZE);

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	Droplet::EngineConfig config{
		Droplet::Graphics::SDL::WindowConfig{
			static_cast<int>(windowSize.x),
			static_cast<int>(windowSize.y),
			{ SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE },
			"Droplet Editor"
		},
		Droplet::UpdateFlags::EditorFlags
	};
	Droplet::Engine engine(config);

	SDL_Window *wnd = engine.GetWindow();
	if (!wnd)
	{
		return 1;
	}
    
	// Set the script path for where the engine will look for scripts.
	Droplet::Script::ScriptSystem::Get().SetScriptPath(ROOT_PATH "/src/Scripts");

	// Setup SDL polling listener
	engine.AddEventListener([](SDL_Event &event)
	{
		// ImGui listener callback
		ImGui_ImplSDL3_ProcessEvent(&event);
	});

	engine.AddEventListener([](SDL_Event &event)
	{
		// Window state event callback
		if (event.type == SDL_EVENT_WINDOW_RESIZED)
		{
			int width = event.window.data1;
			int height = event.window.data2;
			EditorConfig::StoreVec2("Editor.WindowSize", glm::vec2(width, height));
			return;
		}

		if (event.type == SDL_EVENT_WINDOW_MAXIMIZED)
		{
			EditorConfig::StoreBool("Editor.WindowMaximized", true);
			return;
		}

		if (event.type == SDL_EVENT_WINDOW_RESTORED)
		{
			EditorConfig::StoreBool("Editor.WindowMaximized", false);
			return;
		}
	});

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
	static const ImWchar iconsRanges[] = {ICON_MIN_LC, ICON_MAX_16_LC, 0};
	io.Fonts->AddFontFromFileTTF(ROOT_PATH "/Editor/Fonts/lucide.ttf", baseFontSize, &iconsConfig, iconsRanges);
	

	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(ROOT_PATH "/Editor/Fonts/"))
	{
		if (dirEntry.path().extension() == ".ttf" && dirEntry.path().filename().generic_string() != "lucide.ttf")
		{
			io.Fonts->AddFontFromFileTTF(dirEntry.path().generic_string().c_str());   
		}
	}
	
	// ------

	// Restore window maximized state
	{
		bool isMaximized = EditorConfig::LoadBool("Editor.WindowMaximized", false);

		if (isMaximized)
		{
			SDL_MaximizeWindow(wnd);
		}
	}

	// Setup default ImGui Layout
	{
		std::string currLayout = EditorConfig::LoadString("Editor.Layout", "Default");

		try
		{
			UIStyle::LoadLayout(currLayout);
		}
		catch (...)
		{
			if (currLayout != "Default")
			{
				UIStyle::LoadLayout("Default");
			}
		}
	}

	// Setup default ImGui style
	{
		std::string currStyle = EditorConfig::LoadString("Editor.Style", "Default");

		try
		{
			UIStyle::LoadStyle(currStyle);
		}
		catch (...)
		{
			if (currStyle != "Default")
			{
				UIStyle::LoadStyle("Default");
			}
		}
	}

	EditorContext editorContext(engine);

	HierarchyWindow hierarchyWindow(&editorContext);
	NodeInspectorWindow nodeInspectorWindow(&editorContext);
	BehaviourInspector behaviourInspector(&editorContext);
	ScriptsWindow scriptsWindow;
    SavedDataWindow savedDataWindow;
	Resource::ResourceBrowser resourceBrowser(&editorContext);

	Scene::SceneViewWindow sceneViewWindow(&editorContext);
	

	hierarchyWindow.Init();
	nodeInspectorWindow.Init();
	behaviourInspector.Init();
	scriptsWindow.Init();
    savedDataWindow.Init();
	resourceBrowser.Init();
	sceneViewWindow.Init();

	FrameMark;

	bool showStyleEditor = false;

	std::string pendingLayoutChange = "";

	ImGui::SetNextWindowPos(ImVec2(1000.0f, 100.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(600.0f, 600.0f), ImGuiCond_Always);

	// Run main loop
	bool run = true;
	while (run)
	{
		ZoneScopedN("Main Loop"); // NOTE: Scoped Tracy calls should almost always be on the first line of their scope

		EditorConfig::Update();

		if (pendingLayoutChange != "")
		{
			try
			{
				UIStyle::LoadLayout(pendingLayoutChange);
			}
			catch (...)
			{
				if (pendingLayoutChange != "Default")
				{
					UIStyle::LoadLayout("Default");
				}
			}

			pendingLayoutChange = "";
		}

		// ImGui
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();
		ImGui::DockSpaceOverViewport();
		ImGuizmo::BeginFrame();

		bool openLayoutPopup = false;

		// Main toolbar
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::BeginMenu("Layout"))
				{
					if (ImGui::BeginMenu("Set Layout"))
					{
						std::string currLayout = EditorConfig::LoadString("Editor.Layout", "Default");

						std::vector<std::string> layouts;
						UIStyle::GetLayoutNames(layouts);

						for (const auto &layoutName : layouts)
						{
							bool isSelected = (layoutName == currLayout);
							if (ImGui::MenuItem(layoutName.c_str(), nullptr, isSelected))
							{
								if (layoutName != currLayout)
								{
									pendingLayoutChange = layoutName;
								}
							}
						}

						ImGui::EndMenu();
					}

					openLayoutPopup = ImGui::MenuItem("Save Layout");

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

			ImGui::EndMainMenuBar();
		}

		if (openLayoutPopup)
		{
			ImGui::OpenPopup("Save Layout");
		}

		if (ImGui::BeginPopup("Save Layout"))
		{
			static std::string layoutName = "";

			ImGui::InputText("Layout Name", &layoutName);

			// Disable if the layout name is empty
			ImGui::BeginDisabled(layoutName.empty());

			if (ImGui::Button("Confirm"))
			{
				UIStyle::SaveLayout(layoutName);
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndDisabled();

			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
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
				static std::string styleName = "";

				ImGui::InputText("Style Name", &styleName);

				// Disable if the style name is empty
				ImGui::BeginDisabled(styleName.empty());

				if (ImGui::Button("Confirm"))
				{
					UIStyle::SaveStyle(styleName);
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndDisabled();

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
        savedDataWindow.Render();
		resourceBrowser.Render();
		sceneViewWindow.Render();

		switch (engine.Update())
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

		Droplet::Graphics::CameraRenderData camRenderData{ };
		{
			ZoneScopedN("Editor Camera Render Data");

			using namespace Droplet::Graphics::VK;

			IntermediaryStorage &instance = IntermediaryStorage::GetInstance();
			auto &editorCamera = editorContext.GetEditorCamera();
			auto activeScene = editorContext.GetActiveScene();

			camRenderData.camera = static_cast<Droplet::Graphics::Camera>(editorCamera);
			camRenderData.renderTarget = editorCamera.GetTargetIndex();

			auto &renderables = activeScene->GetRenderables();

			Droplet::Math::Frustum frustum = editorCamera.GetFrustum();

			auto visibleComponents = activeScene->GetNodesIntersectingFrustum(frustum);

			std::vector<std::weak_ptr<Droplet::Scene::Component>> componentsToRender;

			// Filter the renderables to only include those that are visible in the editor camera's frustum
			for (const auto &[node, component] : renderables)
			{
				if (!node || !component)
				{
					continue;
				}

				// Check if the node is in the list of visible components
				auto it = std::find_if(visibleComponents.begin(), visibleComponents.end(),
					[&node](const std::weak_ptr<Droplet::Scene::Node> &weakNode) {
						auto sharedNode = weakNode.lock();
						return sharedNode && sharedNode == node;
					});

				if (it != visibleComponents.end())
				{
					componentsToRender.emplace_back(component);
				}
			}

			camRenderData.renderData.reserve(componentsToRender.size());
			for (const auto &componentWeak : componentsToRender)
			{
				auto component = componentWeak.lock();
				if (!component)
				{
					continue;
				}

				camRenderData.renderData.emplace_back(component->GetRenderData());
			}
		}

		switch (engine.Render(&camRenderData))
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
	}

	return 0;
}
