#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_vulkan.h>
#include <ImGui/imgui_impl_sdl3.h>

#include <Engine/EngineFlags.hpp>
#include <Graphics/SDL/Window.hpp>
#include <Graphics/VK/Renderer.hpp>
#include <Resource/ResourceManager.hpp>
#include <ScriptSystem/ScriptSystem.hpp>
#include <SceneSystem/SceneManager.hpp>
#include <SceneSystem/Behaviour.hpp>
#include <GameInput.hpp>
#include <Time.hpp>

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
		Graphics::SDL::WindowConfig WindowConfig;
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

		/// @brief Adds an event listener that will be called when an SDL event is polled.
		/// @param p_listener The event listener to add.
		void AddEventListener(const EventListener& p_listener) { m_eventListeners.push_back(p_listener); }
		
		/// @brief Is meant to enable other classes to use functionality from scenemanager
		/// @return A reference to engines scenemanager
		[[nodiscard]] Scene::SceneManager &GetSceneManager() { return m_sceneManager; }
		
		[[nodiscard]] SDL_Window *GetWindow() const { return m_window.Get(); }
		
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
		
		/// @brief The resource manager instance, should only be one.
		ResourceManager& m_resourceManager { ResourceManager::GetInstance() };

		/// @brief The window instance should only be one.
		Graphics::SDL::Window m_window;

		/// @brief The renderer instance, should only be one.
		Graphics::Renderer m_renderer;

		/// @brief The scenemanager instance, should only be one.
		Scene::SceneManager m_sceneManager;

		/// @brief 
		ImGui_ImplVulkan_InitInfo m_initInfo {};

		/// @brief A list of event listeners that will be called when an SDL event is polled.
		std::vector<EventListener> m_eventListeners;
		
		/// @brief Registers all behaviours owned by the engine.
		void RegisterBehaviours();

		/// @brief Registers all components owned by the engine.
		void RegisterComponents();
	};
}
