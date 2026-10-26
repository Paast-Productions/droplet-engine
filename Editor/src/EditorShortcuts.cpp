#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_vulkan.h>
#include <ImGui/imgui_impl_sdl3.h>
#include <ImGuizmo/ImGuizmo.h>
#include "EditorShortcuts.hpp"
#include <GameInput.hpp>
#include "EditorConfig.hpp"
#include <iostream>
#include "EditorContext.hpp"
#include <SceneSystem/Node.hpp>
#include <SceneSystem/Serialization/SceneSerializer.hpp>
#include <Transform.hpp>

using namespace Droplet;

void Editor::EditorShortcuts::ProcessSceneViewKeyShortcuts(EditorContext &p_editorContext)
{
	GameInput &input = GameInput::Get();

	ImGuizmo::OPERATION noneOperation = (ImGuizmo::OPERATION)0;
	ImGuizmo::OPERATION translationOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::OPERATION rotationOperation = ImGuizmo::ROTATE_X | ImGuizmo::ROTATE_Y | ImGuizmo::ROTATE_Z;
	ImGuizmo::OPERATION scaleOperation = ImGuizmo::SCALE;
	ImGuizmo::OPERATION boundsOperation = ImGuizmo::BOUNDS;
	ImGuizmo::OPERATION universalOperation = ImGuizmo::UNIVERSAL;

	// None
	if (input.KeyPressed(Key::KeyQ))
	{
		EditorConfig::StoreBool("manipulateBounds", false);
		EditorConfig::StoreInt("manipulateOperation", noneOperation);
	}

	// Translation
	if (input.KeyPressed(Key::KeyW))
	{
		EditorConfig::StoreBool("manipulateBounds", false);
		EditorConfig::StoreInt("manipulateOperation", translationOperation);
	}

	// Rotation
	if (input.KeyPressed(Key::KeyE))
	{
		EditorConfig::StoreBool("manipulateBounds", false);
		EditorConfig::StoreInt("manipulateOperation", rotationOperation);
	}

	// Scale
	if (input.KeyPressed(Key::KeyR))
	{
		EditorConfig::StoreBool("manipulateBounds", false);
		EditorConfig::StoreInt("manipulateOperation", scaleOperation);
	}

	// Bounds
	if (input.KeyPressed(Key::KeyT))
	{
		EditorConfig::StoreBool("manipulateBounds", true);
		EditorConfig::StoreInt("manipulateOperation", boundsOperation);
	}

	// Universal
	if (input.KeyPressed(Key::KeyY))
	{
		EditorConfig::StoreBool("manipulateBounds", false);
		EditorConfig::StoreInt("manipulateOperation", universalOperation);
	}

	if (input.KeyPressed(Key::KeyDelete))
	{
		for (auto &node : p_editorContext.GetInteractionState().GetSelectedNodes())
		{
			if (!node)
			{
				continue;
			}

			auto oldParent = node->GetParent();

			if (oldParent)
			{
				oldParent->RemoveChild(node);
			}

			p_editorContext.GetActiveScene()->RemoveRoot(node);
		}

		p_editorContext.GetInteractionState().ClearNodeSelection();
	}
  
	const bool ctrlPressed = input.KeyHeld(Key::KeyLCTRL) || input.KeyHeld(Key::KeyRCTRL);
	
	if (ctrlPressed)
	{
		if (input.KeyPressed(Key::KeyC))
		{
			m_copiedNodes.clear();
			for (auto selectedNode : p_editorContext.GetInteractionState().GetSelectedNodes())
			{
				m_copiedNodes.push_back(selectedNode);
			}
		}

		if (input.KeyPressed(Key::KeyV))
		{
			for (auto node : m_copiedNodes)
			{
				std::println("PASTING DISABLED DUE TO ADVERSE EFFECTS");
				return; // HACK: ABSOLUTELY DO NOT DO THIS. THIS WILL SILENTLY OVERWRITE PREFABS. Disabled for now to avoid mistakes.

				//HACK very hacky fix this to be from a imGui clipboard, and we shouldn't even serialize it to a node
				//HACK save just the nlohmann::json to a clipboard and deserialize that instead
				const std::filesystem::path prefabDirectory = ROOT_PATH "/GameData/Prefabs";
				std::string filePath = (prefabDirectory / (node.lock()->GetName() + ".json")).string();
				//std::string name = node.lock()->GetName() + ".json";
				p_editorContext.SaveNode(filePath,*node.lock());

				glm::vec3 objectPos = p_editorContext.GetEditorCamera().GetPosition() + p_editorContext.GetEditorCamera().GetForward() * 5.0f;
				
				p_editorContext.LoadNodeFromFile(filePath,objectPos);
			}
		}

		if (input.KeyPressed(Key::KeyS))
		{
			auto scene = p_editorContext.GetActiveScene();

			if (scene)
			{
				std::string path = ROOT_PATH "/GameData/Scenes/" + scene->GetName() + ".json";
				p_editorContext.SaveScene(path);
			}
		}
	}
}
