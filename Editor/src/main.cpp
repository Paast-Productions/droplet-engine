#include <Engine/Engine.hpp>
#include "HierarchyWindow.hpp"
#include "NodeInspectorWindow.hpp"
#include "SavedDataWindow.hpp"
#include "ScriptsWindow.hpp"
#include "BehaviourInspector.hpp"
#include <Resource/ResourceBrowser.hpp>
#include <Scene/SceneViewWindow.hpp>
#include <ScriptSystem/ScriptSystem.hpp>
#include <Graphics/VK/TestData.hpp>
#include <tracy/public/tracy/Tracy.hpp>
#include <GameInput.hpp>
#include <EditorContext.hpp>
#include <EditorConfig.hpp>
#include <ImGuizmo/ImGuizmo.h>
#include <ImGui/UIStyle.hpp>
#include <Editor/ImGuiCurve.hpp>
#include <Math/Curves.hpp>
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

    // Editor Scope
    {
        EditorContext editorContext(engine);

        HierarchyWindow hierarchyWindow(&editorContext);
        NodeInspectorWindow nodeInspectorWindow(&editorContext);
        BehaviourInspector behaviourInspector(&editorContext);
        Resource::ResourceBrowser resourceBrowser(&editorContext);
        Scene::SceneViewWindow sceneViewWindow(&editorContext);
        ScriptsWindow scriptsWindow;
        SavedDataWindow savedDataWindow;

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

            hierarchyWindow.Render();
            nodeInspectorWindow.Render();
            behaviourInspector.Render();
            scriptsWindow.Render();
            savedDataWindow.Render();
            resourceBrowser.Render();
            sceneViewWindow.Render();

            // HACK: Temp window for testing ImGuiCurveEdit
            {
                if (ImGui::Begin("Curve Editor Test"))
                {
                    using namespace ImGui;

                    // Use custom curve editor
                    static std::vector<BezierPoint> points = {
                        { { 0.0f, 0.0f }, { 0.5f, 0.0f }, { 0.5f, 0.0f } },
                        { { 1.0f, 1.0f }, { 0.5f, 1.0f }, { 0.5f, 1.0f } }
                    };

                    static ImGuiCurveEditFlags flags = ImGuiCurveEditFlags_None;

                    static ImVec2 curveSize(350, 350);
                    static ImRect padding(48, 24, 20, 24);
                    static ImRect curveRect(0, 0, 1, 1);
                    static ImVec2i gridLines(4, 4);
                    static float thickness = 3.0f;

                    if (TreeNode("Flags"))
                    {
                        std::vector<std::string> flagNames = {
                            "Linear", "Quadratic", "Jointed", "Force Span Width", "Force Injective",
                            "Read Only", "No Labels", "No Points", "Clamp X", "Clamp Y"
                        };

                        // Flag checkboxes
                        for (int i = 0; i < flagNames.size(); i++)
                        {
                            bool flagSet = (flags & (1 << i)) != 0;
                            if (Checkbox(flagNames[i].c_str(), &flagSet))
                            {
                                if (flagSet)
                                    flags |= (1 << i);
                                else
                                    flags &= ~(1 << i);
                            }
                        }

                        TreePop();
                    }

                    SliderFloat("Thickness##CurveEditor", &thickness, 0.1f, 10.0f);
                    DragFloat2("Size##CurveEditor", &curveSize.x, 1.0f, 1.0f, 0.0f, "%.0f");

                    if (DragInt2("Grid Lines##CurveEditor", &gridLines.x, 0.05f))
                    {
                        gridLines.x = std::max(gridLines.x, 0);
                        gridLines.y = std::max(gridLines.y, 0);
                    }

                    DragFloat4("Padding##CurveEditor", &padding.Min.x, 0.01f, 0.0f, 0.0f, "%.2f");

                    DragFloat4("Bounds##CurveEditor", &curveRect.Min.x, 0.01f, 0.0f, 0.0f, "%.2f");


                    Separator();
                    CurveEdit("##CurveEditor", &points, curveSize, curveRect, thickness, padding, gridLines, flags);

                    // Test sampling using Curves.h
                    if (TreeNode("Sampling"))
                    {
                        using namespace Droplet::Math::Curves;

                        Curve<glm::vec2> curve;
                        curve.type = (flags & ImGuiCurveEditFlags_Linear) ? CurveType::Linear : ((flags & ImGuiCurveEditFlags_Quadratic) ? CurveType::BezierQuadratic : CurveType::BezierCubic);

                        curve.points.resize(points.size());
                        std::memcpy(curve.points.data(), points.data(), points.size() * sizeof(BezierPoint));

                        static bool injective = false;
                        static float sampleT = 0.5f;

                        BeginChild("Curve Sampling", curveSize, true);
                        {
                            glm::vec2 sampleValue{};

                            if (injective)
                            {
                                float sampleY = curve.SampleInjective(sampleT);
                                sampleValue = { sampleT, sampleY };
                            }
                            else
                            {
                                sampleValue = curve.SamplePoint(sampleT);
                            }

                            ImVec2 windowPos = ImGui::GetWindowPos();
                            ImDrawList *drawList = ImGui::GetWindowDrawList();

                            ImVec2 samplePos = { sampleValue.x, 1.0f - sampleValue.y };
                            samplePos *= curveSize;

                            drawList->AddCircleFilled(windowPos + samplePos, thickness * 1.6f, IM_COL32(255, 255, 255, 255));
                        }
                        EndChild();

                        // Sample slider, same width as child window
                        SetNextItemWidth(curveSize.x);
                        SliderFloat("##Sample T", &sampleT, 0.0f, 1.0f);
                        Checkbox("Injective Sampling##CurveSampling", &injective);

                        TreePop();
                    }

                }
                ImGui::End();
            }

            switch (engine.Update(editorContext.GetEditorCamera()))
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
    }

    return 0;
}
