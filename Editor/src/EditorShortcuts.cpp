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
	ImGuizmo::OPERATION noneOperation = (ImGuizmo::OPERATION)0;
	ImGuizmo::OPERATION translationOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::OPERATION rotationOperation = ImGuizmo::ROTATE_X | ImGuizmo::ROTATE_Y | ImGuizmo::ROTATE_Z;
	ImGuizmo::OPERATION scaleOperation = ImGuizmo::SCALE;
	ImGuizmo::OPERATION boundsOperation = ImGuizmo::BOUNDS;
	ImGuizmo::OPERATION universalOperation = ImGuizmo::UNIVERSAL;

	// None
	if (GameInput::Get().KeyPressed(Droplet::Key::Q))
	{
		EditorConfig::StoreBool("manipulateBounds", false);
		EditorConfig::StoreInt("manipulateOperation", noneOperation);
	}

	// Translation
	if (GameInput::Get().KeyPressed(Droplet::Key::W))
	{
		EditorConfig::StoreBool("manipulateBounds", false);
		EditorConfig::StoreInt("manipulateOperation", translationOperation);
	}

	// Rotation
	if (GameInput::Get().KeyPressed(Droplet::Key::E))
	{
		EditorConfig::StoreBool("manipulateBounds", false);
		EditorConfig::StoreInt("manipulateOperation", rotationOperation);
	}

	// Scale
	if (GameInput::Get().KeyPressed(Droplet::Key::R))
	{
		EditorConfig::StoreBool("manipulateBounds", false);
		EditorConfig::StoreInt("manipulateOperation", scaleOperation);
	}

	// Bounds
	if (GameInput::Get().KeyPressed(Droplet::Key::T))
	{
		EditorConfig::StoreBool("manipulateBounds", true);
		EditorConfig::StoreInt("manipulateOperation", boundsOperation);
	}

	// Universal
	if (GameInput::Get().KeyPressed(Droplet::Key::Y))
	{
		EditorConfig::StoreBool("manipulateBounds", false);
		EditorConfig::StoreInt("manipulateOperation", universalOperation);
	}
}
