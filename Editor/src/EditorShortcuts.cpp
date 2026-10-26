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

	const bool ctrlPressed = GameInput::Get().KeyHeld(Droplet::Key::KeyLCTRL) || GameInput::Get().KeyHeld(Droplet::Key::KeyRCTRL);
	
	if (ctrlPressed)
	{
		if (GameInput::Get().KeyPressed(Droplet::Key::KeyC))
		{
			m_copiedNodes.clear();
			for (auto selectedNode : p_editorContext.GetInteractionState().GetSelectedNodes())
			{
				m_copiedNodes.push_back(selectedNode);
			}

		}

		if (GameInput::Get().KeyPressed(Droplet::Key::KeyV))
		{
			for (auto node : m_copiedNodes)
			{
				//HACK very hacky fix this to be from a imGui clipboard, and we shouldn't even serialize it to a node
				//HACK save just the nlohmann::json to a clipboard and deserialize that instead
				const std::filesystem::path prefabDirectory = ROOT_PATH "/GameData/Prefabs";
				std::string filePath = (prefabDirectory / (node.lock()->GetName() + ".json")).string();
				//std::string name = node.lock()->GetName() + ".json";
				p_editorContext.SaveNode(filePath,*node.lock());

				glm::vec3 objectPos = p_editorContext.GetEditorCamera().GetPosition() + p_editorContext.GetEditorCamera().GetForward();
				
				p_editorContext.LoadNodeFromFile(filePath,objectPos);

			}
			
		}
	}
}
