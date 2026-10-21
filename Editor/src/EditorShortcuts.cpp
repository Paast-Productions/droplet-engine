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
	ImGuizmo::OPERATION translationOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::OPERATION rotationOperation = ImGuizmo::ROTATE_X | ImGuizmo::ROTATE_Y | ImGuizmo::ROTATE_Z;
	ImGuizmo::OPERATION scaleOperation = ImGuizmo::SCALE;
	ImGuizmo::OPERATION universalOperation = translationOperation | rotationOperation;

	// Will be "Q" eventually, Q is busy
	// Full tranform manipulation
	if (GameInput::Get().KeyPressed(Droplet::Key::Key1))
	{
		EditorConfig::StoreBool("manipulateBounds", true);
		EditorConfig::StoreInt("manipulateOperation", universalOperation);
	}

	// Translation
	if (GameInput::Get().KeyPressed(Droplet::Key::KeyW))
	{
		EditorConfig::StoreBool("manipulateBounds", false);
		EditorConfig::StoreInt("manipulateOperation", translationOperation);
	}

	// Rotation
	if (GameInput::Get().KeyPressed(Droplet::Key::KeyE))
	{
		EditorConfig::StoreBool("manipulateBounds", false);
		EditorConfig::StoreInt("manipulateOperation", rotationOperation);
	}

	// Scale
	if (GameInput::Get().KeyPressed(Droplet::Key::KeyR))
	{
		EditorConfig::StoreBool("manipulateBounds", true);
		EditorConfig::StoreInt("manipulateOperation", scaleOperation);
	}
}
