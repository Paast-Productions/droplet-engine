#pragma once
#include "InteractionState.hpp"
#include "../../include/SceneSystem/Scene.hpp"
#include <Scene/EditorCamera.hpp>
#include <Scene/EditorCameraController.hpp>
#include <Graphics/VK/Renderer.hpp>
#include <Engine/Engine.hpp>
namespace Droplet::Editor
{
	/// @brief A class that holds the information of active Scene, Renderer, EditorCamera and controller 
	class EditorContext 
	{
	public:
		/// @brief Constructor that holds a reference to the rendering pipeline
		/// @param p_engine The renderer of the rendering pipeline
		EditorContext(Droplet::Engine &p_engine);

		
		Droplet::Engine &GetEngine();
		
		/// @breif Get the renderer
		/// @returns A custom renderer class
		Droplet::Graphics::Renderer &GetRenderer();
		
		Droplet::ResourceManager &GetResourceManager();

		/// @breif Get the EditorCamera
		/// @returns The Camera of the sceneView
		Scene::EditorCamera &GetEditorCamera();
		/// @brief Get the EditorCameraController
		/// @returns The class that controls the camera movement in sceneView
		Scene::EditorCameraController &GetEditorCameraController();
		/// @brief Get the InteractionState
		/// @returns InteractionState
		InteractionState &GetInteractionState();

		/// @brief Get the active scene
		/// @returns A scene from scenemanager
		std::shared_ptr<Droplet::Scene::Scene> GetActiveScene();
		/// @breif Lets the context set a new active scene
		void SetActiveScene(std::string p_sceneName);

	private:
		Droplet::Engine &m_engine;
		
		///
		//Droplet::Scene::SceneManager &m_sceneManager;
		//Droplet::Graphics::Renderer &m_renderer;

		InteractionState m_interactionState;
		Scene::EditorCamera m_editorCamera;
		Scene::EditorCameraController m_editorCameraController;
		std::shared_ptr<Droplet::Scene::Scene> m_selectedScene;

	};
}