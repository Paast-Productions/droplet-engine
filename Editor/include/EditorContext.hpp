#pragma once
#include "Scene/EditorCamera.hpp"
#include "InteractionState.hpp"
#include "../../include/SceneSystem/Scene.hpp"
namespace Droplet::Editor
{
	class EditorContext {
	public:
		EditorContext();

		Scene::EditorCamera &GetEditorCamera();
		InteractionState &GetInteractionState();

		std::shared_ptr<Droplet::Scene::Scene> GetScene();
		void SetScene(std::shared_ptr<Droplet::Scene::Scene> p_scene);

	private:

		Scene::EditorCamera m_editorCamera;
		InteractionState m_interactionState;
		std::shared_ptr<Droplet::Scene::Scene> m_scene;
	};
}