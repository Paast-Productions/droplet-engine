#pragma once
#include "InteractionState.hpp"
#include "../../include/SceneSystem/Scene.hpp"
#include <Scene/EditorCamera.hpp>
#include <Scene/EditorCameraController.hpp>
#include <Graphics/VK/Renderer.hpp>
#include <Engine/Engine.hpp>
#include <SceneSystem/Node.hpp>
namespace Droplet::Editor
{

	

	/// @brief A class that holds the information of active Scene, Renderer, EditorCamera and controller 
	class EditorContext 
	{
	public:
		/// @brief Constructor that holds a reference to the rendering pipeline
		/// @param p_engine The renderer of the rendering pipeline
		EditorContext(Droplet::Engine &p_engine);

		/// @brief Get the engine.
		/// @return A reference to the engine that this context is associated with.
		[[nodiscard]] Droplet::Engine &GetEngine();
		
		/// @breif Get the renderer
		/// @returns A custom renderer class
		[[nodiscard]] Droplet::Graphics::Renderer &GetRenderer();
		
		/// @brief Get the resource manager.
		/// @return A reference to the resource manager that this context is associated with.
		[[nodiscard]] Droplet::ResourceManager &GetResourceManager();

		/// @breif Get the EditorCamera
		/// @returns The Camera of the sceneView
		[[nodiscard]] Scene::EditorCamera &GetEditorCamera();

		/// @brief Get the EditorCameraController
		/// @returns The class that controls the camera movement in sceneView
		[[nodiscard]] Scene::EditorCameraController &GetEditorCameraController();

		/// @brief Get the InteractionState
		/// @returns InteractionState
		[[nodiscard]] InteractionState &GetInteractionState();

		/// @brief Get the active scene
		/// @returns A scene from scenemanager
		[[nodiscard]] std::shared_ptr<Droplet::Scene::Scene> GetActiveScene(); // TODO: Make weak pointer

		/// @breif Lets the context set a new active scene
		/// @param p_sceneName The name of the scene to set as active
		void SetActiveScene(std::string p_sceneName);

		void SaveScene(const std::string &filepath);

		void LoadFromFile( std::string &p_filepath);

		void SaveNode(const std::string &filepath, const Droplet::Scene::Node &p_node);

		void LoadNodeFromFile(std::string &filepath);
	private:
		Droplet::Engine &m_engine;
		InteractionState m_interactionState;
		Scene::EditorCamera m_editorCamera;
		Scene::EditorCameraController m_editorCameraController;
		std::shared_ptr<Droplet::Scene::Scene> m_selectedScene; // TODO: Make weak pointer
	};
}