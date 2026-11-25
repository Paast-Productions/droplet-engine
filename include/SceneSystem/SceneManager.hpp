#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <json/json.hpp>
#include <Audio/AudioSystem.hpp>
#include <Core/ThreadSafeQueue.hpp>

namespace Droplet::Graphics
{
	struct CameraRenderData;
}

namespace Droplet::Game 
{ 
    class ItemRegistry; 
}

namespace Droplet::Scene
{
    class Scene;
    class Behaviour;
    class Node;
	class CameraComponent;

    /// @brief Manages the Scenes used by the application.
    ///
    /// SceneManager is responsible for loading, unloading, activating,
    /// and deactivating Scenes. It also updates and renders all
    /// currently active Scenes.
    ///
    /// Loaded Scenes are stored using shared ownership, while the list
    /// of active Scenes uses weak references to avoid additional ownership.
    class SceneManager
    {
    public:

        /// @brief Constructs an empty SceneManager.
        SceneManager();

        /// @brief Destroys the SceneManager.
        ~SceneManager();

        /// @brief Loads a Scene with the specified name.
        ///
        /// The Scene is created using the SceneFactory, loaded, and stored
        /// in the SceneManager.
        ///
        /// @param p_name Name of the Scene to load.
        ///
        /// @throws std::runtime_error if a Scene with the specified name
        /// already exists or if the SceneFactory fails to create the Scene.
        std::weak_ptr<Scene> LoadScene(const std::string &p_name);

        /// @brief Unloads a Scene with the specified name.
        ///
        /// The Scene is deactivated if necessary, unloaded, and removed
        /// from the SceneManager.
        ///
        /// @param p_name Name of the Scene to unload.
        ///
        /// @throws std::runtime_error if a Scene with the specified name
        /// does not exist.
        void UnloadScene(const std::string &p_name);

        /// @brief Activates a loaded Scene.
        ///
        /// An active Scene is updated and rendered by the SceneManager.
        ///
        /// @param p_name Name of the Scene to activate.
        ///
        /// @throws std::runtime_error if the Scene does not exist, is not
        /// loaded, or is already active.
        void ActivateScene(const std::string &p_name);

        /// @brief Deactivates an active Scene.
        ///
        /// A deactivated Scene remains loaded but is no longer updated
        /// or rendered.
        ///
        /// @param p_name Name of the Scene to deactivate.
        ///
        /// @throws std::runtime_error if the Scene does not exist, is not
        /// loaded, or is already inactive.
        void DeactivateScene(const std::string &p_name);

        /// @brief Gets a loaded Scene by name.
        ///
        /// @param p_name Name of the Scene to retrieve.
        ///
        /// @return A shared pointer to the Scene, or nullptr if the
        /// Scene is not loaded.
        [[nodiscard]] std::shared_ptr<Scene> GetScene(const std::string &p_name) const; // TODO: (SCENE) Make weak pointer

        /// @brief Gets the currently active Scenes.
        ///
        /// The returned list contains weak references to the Scenes that
        /// are currently active. Callers must lock the weak pointers before
        /// accessing the Scenes.
        ///
        /// @return A constant reference to the list of active Scene
        /// weak pointers.
        [[nodiscard]] const std::vector<std::weak_ptr<Scene>> &GetActiveScenes() const;

        /// @brief Updates all active Scenes.
        void Update();

        /// @brief Returns Renderdata for all active scenes
        [[nodiscard]] std::vector<Droplet::Graphics::CameraRenderData> GetRenderData();

		/// @brief Performs post-rendering operations for all active Scenes.
		void PostRender();

        /// @brief Seralizes and writes a scene to a file
        /// @param p_scene The scene that is to serialize
        /// @param p_filePath The json file to write to
        void SerializeToFile(std::shared_ptr<Scene> p_scene, const std::string& p_filePath); // TODO: (SCENE) Make weak pointer

        /// @brief Loads a scene from a file
        /// @param p_filePath The name of the file the scene is serialized in
        void LoadFromFile(const std::string &p_filePath);

        /// @brief Loads a scene from a file asynchronously.
        ///
        /// The file is read, parsed, and deserialized on the thread pool. Not all
        /// systems are thread safe, so deserialization may race with the main thread.
        /// The Scene is registered during the next call to Update(), on the calling
        /// thread, and then p_onLoaded is invoked. Failures are reported to stderr,
        /// the Scene is not registered, and p_onLoaded receives nullptr.
        /// @param p_filePath The name of the file the scene is serialized in
        /// @param p_onLoaded Optional callback invoked on the main thread when loading finishes
        void LoadFromFileAsync(const std::string &p_filePath, std::function<void(std::shared_ptr<Scene>)> p_onLoaded = nullptr);

        /// @return True if any scene loaded with LoadFromFileAsync has not yet been created.
        [[nodiscard]] bool HasPendingAsyncLoads() const;

        /// @brief Checks whether all scenes are fully loaded and every resource they depend on has finished loading.
        /// @return True if no async scene loads are pending and every scene reports its resources as loaded.
        [[nodiscard]] bool IsLoadingComplete() const;

        /// @brief Serializes a lonesome node to a josn file
        /// @param p_node the node to be serialized
        /// @param p_filePath the filepath
        void SerializeNodeToFile(Node &p_node, const std::string &p_filePath);

		/// @brief Gets the list of registered behaviours.
		/// @return A constant reference to the list of registered behaviours.
		const std::vector<std::shared_ptr<Behaviour>> &GetBehaviours() { return m_behaviours; } // TODO: (SCENE) Make weak pointer

		/// @brief Starts a behaviour of the specified type.
        /// @param p_behaviourTypeName The type name of the behaviour to start.
		/// @return A shared pointer to the started Behaviour.
		/// @throws std::runtime_error if the behaviour type is not registered or cannot be started.
        [[nodiscard]] std::weak_ptr<Behaviour> StartBehaviour(const std::string &p_behaviourTypeName);

        /// @brief Shuts down a behaviour.
		/// @param p_behaviour A weak pointer to the behaviour to shut down.
		void ShutdownBehaviour(std::weak_ptr<Behaviour> p_behaviour);
        
		/// @brief Gets the main camera of all active scenes.
		/// @return A weak pointer to the CameraComponent at the end of the camera stack.
        [[nodiscard]] std::weak_ptr<CameraComponent> GetMainCamera() const;

        [[nodiscard]] const std::unordered_map<std::string, std::shared_ptr<Scene>> &GetScenes() const { return m_scenes; }
        
        // HACK: Game system hardcoded into engine
        void SetItemRegistry(const Droplet::Game::ItemRegistry *p_reg) { m_itemRegistry = p_reg; }
        [[nodiscard]] const Droplet::Game::ItemRegistry *GetItemRegistry() const { return m_itemRegistry; }

    private:
        std::unordered_map<std::string, std::shared_ptr<Scene>> m_scenes;
        std::vector<std::weak_ptr<Scene>> m_activeScenes;
        const Droplet::Game::ItemRegistry *m_itemRegistry = nullptr;

		std::vector<std::shared_ptr<Behaviour>> m_behaviours;
        
        struct AsyncSceneLoadResult
        {
            std::string filePath;
            std::string sceneName;
            std::shared_ptr<Scene> scene;
            std::string error;
            std::function<void(std::shared_ptr<Scene>)> onLoaded;
        };

        /// @brief Creates and deserializes a Scene from parsed json. Does not register the Scene.
        [[nodiscard]] std::shared_ptr<Scene> CreateSceneFromJson(const nlohmann::json &p_json);
        /// @brief Registers the Scenes for all completed asynchronous loads. Must run on the main thread.
        void FinalizeAsyncSceneLoads();

        ThreadSafeQueue<AsyncSceneLoadResult> m_asyncSceneReadResults;
        std::atomic<std::size_t> m_pendingAsyncSceneLoads = 0;

        /// @brief Prepares all active Scenes for rendering.
        void PrepareRender();
    };
}