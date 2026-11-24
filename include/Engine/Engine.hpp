#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <unordered_map>
#include <vector>

// TEMP TEMP TEMP TEMP
#include "Integration/Platform/SDLWindow.hpp"
#include "Integration/Platform/SDLVulkan.hpp"
#include "Integration/Vulkan/Frame/Renderer.hpp"
#ifdef _EDITOR
#include "Integration/App/ImGuiFrame.hpp"
#include <ImGui/imgui.h>
#endif

#include <Graphics/Common/RenderData.hpp>
#include <Engine/EngineFlags.hpp>
#include <ScriptSystem/ScriptSystem.hpp>
#include <SceneSystem/SceneManager.hpp>
#include <GameInput.hpp>
#include <Audio/AudioSystem.hpp>
#include <SceneSystem/Items/ItemRegistry.hpp>

namespace Droplet
{
	enum class DROPLET_RETURNTYPE
	{
		OK,
		EXIT,
		EXIT_ERROR
	};
	
	/// @brief EngineConfig is used to configure engine functionality on startup.
	struct EngineConfig
	{
		Platform::SDL::Window::Config WindowConfig;
		UpdateFlags UpdateFlags{ UpdateFlags::DefaultFlags };
	};
	
	/// @brief Engine owns all long-lived engine subsystems. The engine is responsible for initializing the subsystems and to destroy them.
	/// @details The engine class should not be bloated with various different functions from subsystems, rather it should give access to the 
	/// subsystems via some getter function. The engine class is responsible for updating subsystems.
	class Engine
	{
	public:
		using EventListener = std::function<void(SDL_Event &)>;

		Engine(EngineConfig p_config);
		~Engine();

		void Shutdown();
		
		/// @brief Will update the subsystems. Is needed to run anything.
		/// @return The droplet returntype can be used to detemine if the update was successful or not.
		[[nodiscard]] DROPLET_RETURNTYPE Update();

       /// @brief Starts the ImGui frame; call before any ImGui window
       void BeginFrame();

       /// @brief Renders the scene's cameras and `p_cameras` (the editor's), then the ImGui frame
       [[nodiscard]] DROPLET_RETURNTYPE Render(std::span<const Graphics::CameraRenderData> p_cameras);

       /// @brief Adds an event listener that will be called when an SDL event is polled.
       /// @param p_listener The event listener to add.
       void AddEventListener(const EventListener &p_listener) { m_eventListeners.push_back(p_listener); }

       /// @brief Is meant to enable other classes to use functionality from scenemanager
       /// @return A reference to engines scenemanager
       [[nodiscard]] Scene::SceneManager &GetSceneManager() { return m_sceneManager; }

       /// @brief Getter for the window of the engine
       /// @return An SDL window handle
       [[nodiscard]] SDL_Window *GetWindow() { return m_window.GetSDLWindow(); }

    private:

       /// @brief Setter class with the sole purpose of setting the update flags for the engine.
       ///
       /// This is done to ensure that the update flags are set before any other subsystems are
       /// initialized while still following RAII principles.
       /// @note Does this make you happy, Christoffer?
       class UpdateFlagsSetter
       {
       public:
          UpdateFlagsSetter() = delete;

          UpdateFlagsSetter(UpdateFlags p_flags)
          {
             EngineFlagsOwner::SetUpdateFlags(p_flags);
          }
       };

       UpdateFlagsSetter m_updateFlagsSetter;

       // == Window and rendering == constructed in this order, destroyed in reverse
       Platform::SDL::Window m_window;
       Platform::SDL::VulkanSurfaceProvider m_surfaceProvider;
#ifdef _EDITOR
       ImGuiFrame m_imguiFrame; ///< the ImGui context, outlives the renderer's ImGui backend
#endif
       Vulkan::Renderer m_renderer;

       /// @brief Registry containing the game's registered item definitions.
       Game::ItemRegistry m_itemRegistry; // HACK: Game systems do not go in Engine, this would work as a behaviour

		/// @brief The scenemanager instance, should only be one.
		Scene::SceneManager m_sceneManager;

		/// @brief A list of event listeners that will be called when an SDL event is polled.
		std::vector<EventListener> m_eventListeners;
		

		/// @brief Registers all behaviours owned by the engine.
		void RegisterBehaviours();

       /// @brief Registers all components owned by the engine.
       void RegisterComponents();
    };
}
