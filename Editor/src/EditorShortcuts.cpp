#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_vulkan.h>
#include <ImGui/imgui_impl_sdl3.h>
#include <ImGuizmo/ImGuizmo.h>
#include "EditorShortcuts.hpp"
#include <GameInput.hpp>
#include "EditorConfig.hpp"
#include <iostream>

using namespace Droplet;

void Editor::EditorShortcuts::ProcessSceneViewKeyShortcuts()
{
	// Will be "Q" eventually, Q is busy
	// Full tranform manipulation
	if (GameInput::Get().KeyPressed(Droplet::Key::Key1))
	{
		EditorConfig::StoreBool("manipulateBounds", true);
		EditorConfig::StoreInt("manipulateOperation", 
			ImGuizmo::TRANSLATE |
			ImGuizmo::ROTATE_X | ImGuizmo::ROTATE_Y | ImGuizmo::ROTATE_Z |
			ImGuizmo::BOUNDS);
	}

	// Translation
	if (GameInput::Get().KeyPressed(Droplet::Key::KeyW))
	{
		EditorConfig::StoreBool("manipulateBounds", false);
		EditorConfig::StoreInt("manipulateOperation", ImGuizmo::TRANSLATE);
	}

	// Rotation
	if (GameInput::Get().KeyPressed(Droplet::Key::KeyE))
	{
		EditorConfig::StoreBool("manipulateBounds", false);
		EditorConfig::StoreInt("manipulateOperation", ImGuizmo::ROTATE_X | ImGuizmo::ROTATE_Y | ImGuizmo::ROTATE_Z);
	}

	// Scale
	if (GameInput::Get().KeyPressed(Droplet::Key::KeyR))
	{
		EditorConfig::StoreBool("manipulateBounds", true);
		EditorConfig::StoreInt("manipulateOperation", ImGuizmo::BOUNDS);
	}
}
