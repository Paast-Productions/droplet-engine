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
#include "EditorClipboard.hpp"

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
			CopySelectedNodes(p_editorContext);
		}

		if (input.KeyPressed(Key::KeyV))
		{
			PasteNodes(p_editorContext);
		}

		if (input.KeyPressed(Key::KeyS) && !p_editorContext.GetIsPlayMode())
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

void Droplet::Editor::EditorShortcuts::CopySelectedNodes(EditorContext &p_editorContext)
{
	Droplet::Scene::SceneSerializer serializer;

	std::vector<nlohmann::json> serializedNodes;
	serializedNodes.reserve(p_editorContext.GetInteractionState().GetSelectedNodes().size());

	for (const std::shared_ptr<Droplet::Scene::Node>& node : p_editorContext.GetInteractionState().GetSelectedNodes())
	{
		if (!node)
		{
			continue;
		}
		nlohmann::json nodeJson = serializer.SerializeNode(*node);
		serializedNodes.push_back(std::move(nodeJson));
	}
	Editor::EditorClipboard::CopyNodes(serializedNodes);
}

void Droplet::Editor::EditorShortcuts::PasteNodes(EditorContext &p_editorContext)
{
	Droplet::Scene::SceneSerializer serializer;
	const std::optional<std::vector<nlohmann::json>> clipboardNodes = Editor::EditorClipboard::PasteNodes();

	if (!clipboardNodes.has_value())
	{
		return;
	}

	auto scene = p_editorContext.GetActiveScene();
	glm::vec3 objectPos = p_editorContext.GetEditorCamera().GetPosition() + p_editorContext.GetEditorCamera().GetForward() * 5.0f;

	for (const nlohmann::json &nodeJson : *clipboardNodes)
	{
		serializer.DeserializeLonsesomeNode(nodeJson, *scene, objectPos);
	}

}
