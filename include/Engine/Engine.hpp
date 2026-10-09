#pragma once

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_vulkan.h>
#include <ImGui/imgui_impl_sdl3.h>
#include <Graphics/VK/Renderer.hpp>
#include <Graphics/SDL/Window.hpp>
#include <ScriptSystem/ScriptSystem.hpp>
#include <SceneSystem/SceneManager.hpp>
#include <GameInput.hpp>
#include <Time.hpp>
#include <functional>
#include <Engine/EngineFlags.hpp>
#include <resource/ResourceManager.hpp>

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
		
		/// @brief Will update the subsystems. Is needed to run anything.
		/// @return The droplet returntype can be used to detemine if the update was successful or not.
		[[nodiscard]] DROPLET_RETURNTYPE Update();
		
		/// @brief the purpose of GetWindow is to let other systems use the window. 
		/// Mainly the editor and the game itself needs access to the window.
		/// @return An SDL window
		[[nodiscard]] SDL_Window *GetWindow();

		/// @brief 
		/// HACK HACK HACK 
		/// DO NOT USE THIS FUNCTION, IT WILL BE REMOVED VERY SOON.
		/// IT ONLY EXISTS TO MAKE SURE TEMPORARY IMGUI CODE IN MAIN CAN RUN. 
		/// HACK HACK HACK
		/// @return Renderer
		[[nodiscard]] Graphics::Renderer &TEMP_GetRenderer();

		/// @brief Is meant to enable other classes to use functionality from scenemanager
		/// @return A reference to engines scenemanager
		[[nodiscard]] Scene::SceneManager &GetSceneManager();

		/// @brief Adds an event listener that will be called when an SDL event is polled.
		/// @param p_listener The event listener to add.
		void AddEventListener(EventListener p_listener);
		
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

		/// @brief The renderer instance, should only be one.
		Graphics::Renderer m_renderer;

		ResourceManager m_resourceManager;

		/// @brief The scenemanager instance, should only be one.
		Scene::SceneManager m_sceneManager;

		/// @brief 
		ImGui_ImplVulkan_InitInfo m_initInfo;

		/// @brief A list of event listeners that will be called when an SDL event is polled.
		std::vector<EventListener> m_eventListeners;
	};
}
