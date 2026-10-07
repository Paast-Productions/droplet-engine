#pragma once
#include "InteractionState.hpp"
#include "../../include/SceneSystem/Scene.hpp"
#include <Scene/EditorCamera.hpp>
#include <Scene/EditorCameraController.hpp>
namespace Droplet::Editor
{
	class EditorContext {
	public:
		EditorContext();

		Scene::EditorCamera &GetEditorCamera();
		Scene::EditorCameraController &GetEditorCameraController();
		InteractionState &GetInteractionState();

		std::shared_ptr<Droplet::Scene::Scene> GetScene();
		void SetScene(std::shared_ptr<Droplet::Scene::Scene> p_scene);

	private:

		Scene::EditorCamera m_editorCamera;
		Scene::EditorCameraController m_editorCameraController;
		InteractionState m_interactionState;
		std::shared_ptr<Droplet::Scene::Scene> m_scene;
	};
}