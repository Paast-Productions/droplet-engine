#pragma once
#include "InteractionState.hpp"
#include "SceneSystem/Scene.hpp"
#include <Scene/EditorCamera.hpp>
#include <Scene/EditorCameraController.hpp>
#include <Graphics/VK/Renderer.hpp>
#include <Engine/Engine.hpp>
#include <SceneSystem/Node.hpp>

namespace Droplet::Scene
{
	class CameraComponent;
}

namespace Droplet::Editor
{

	struct AudioMixerEditorState
	{
		std::optional <std::uint64_t> selectedAudioBusID;
	};

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
		
		/// @brief Get the EditorCamera
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

		/// @brief Get the mixer
		/// @return the mixer
		//TODO[[nodiscard]] Audio::AudioSystem &GetAudioSystem();

		AudioMixerEditorState &GetAudioMixerState() { return m_selectedBus; }
		/// @breif Lets the context set a new active scene
		/// @param p_sceneName The name of the scene to set as active
		void SetActiveScene(std::string p_sceneName);

		/// @brief Creates a fresh new scene
		/// @param p_sceneName scene name
		void CreateNewScene(std::string p_sceneName);
		
		/// @brief Serializes the scene
		/// @param filepath filepath
		void SaveScene(const std::string &filepath);

		/// @brief Loads a scene from a file
		/// @param p_filepath Filepath
		void LoadFromFile(const std::string &p_filepath);
		/// @brief Serializes a node
		/// @param filepath Filepath to save node to
		/// @param p_node The node to serialize
		void SaveNode(const std::string &filepath, const Droplet::Scene::Node &p_node);

		/// @brief Loads a node from a pre-existing file
		/// @param filepath File
		void LoadNodeFromFile(std::string &filepath, std::optional<glm::vec3> p_overridePosition = std::nullopt);

		/// @brief Sets the game camera for the editor context.
		/// This determines which camera is used for rendering the game view within the editor.
		/// @param p_gameCamera A weak pointer to the CameraComponent that should be used as the game camera.
		void SetGameCamera(std::weak_ptr<Droplet::Scene::CameraComponent> p_gameCamera) { m_gameCamera = p_gameCamera; }

		/// @brief Gets the current game camera for the editor context.
		/// @return A weak pointer to the CameraComponent that is currently set as the game camera.
		[[nodiscard]] std::weak_ptr<Droplet::Scene::CameraComponent> GetGameCamera() { return m_gameCamera; }

	private:
		Droplet::Engine &m_engine;
		InteractionState m_interactionState;
		Scene::EditorCamera m_editorCamera;
		Scene::EditorCameraController m_editorCameraController;
		std::shared_ptr<Droplet::Scene::Scene> m_selectedScene; // TODO: Make weak pointer

		std::weak_ptr<Droplet::Scene::CameraComponent> m_gameCamera{};
		//Audio::AudioSystem m_audioSystem;
		AudioMixerEditorState m_selectedBus;
	};
}