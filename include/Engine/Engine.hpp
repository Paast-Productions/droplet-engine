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

namespace Droplet
{
	/// @brief UpdateFlags are used to control the execution of the frame loop.
	/// @details Certain functionality may be skipped or modified in case the default frame loop logic is not desired.
	/// For example, node + behaviour logic and automatic rendering should be disabled in the Editor.
	enum class UpdateFlags : std::uint32_t
	{
		None					= 0,		// No flags set
		SkipNodeLogic			= 1 << 0,	// Skip node logic
		SkipBehaviourLogic		= 1 << 1,	// Skip behaviour logic
		SkipScriptLogic			= 1 << 2,	// Skip script logic
		SkipAutoRender			= 1 << 3,	// Skip implicitly rendering cameras in active scenes. Cameras must be submitted to the renderer manually.
		// Add more flags as needed

		EditorFlags				= SkipNodeLogic | SkipBehaviourLogic | SkipScriptLogic | SkipAutoRender,
		DefaultFlags			= None
	};

	enum class DROPLET_RETURNTYPE
	{
		OK,
		EXIT,
		EXIT_ERROR
	};
	
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

		/// @brief DO NOT USE THIS FUNCTION, IT WILL BE REMOVED VERY SOON.
		/// IT ONLY EXISTS TO MAKE SURE TEMPORARY IMGUI CODE IN MAIN CAN RUN.
		/// @return Renderer
		[[nodiscard]] Graphics::Renderer &TEMP_GetRenderer();

		/// @brief Is meant to enable other classes to use functionality from scenemanager
		/// @return A reference to engines scenemanager
		[[nodiscard]] Scene::SceneManager &GetSceneManager();

		/// @brief Adds an event listener that will be called when an SDL event is polled.
		/// @param p_listener The event listener to add.
		void AddEventListener(EventListener p_listener);

		/// @brief Gets the update flags for the engine.
		[[nodiscard]] UpdateFlags GetUpdateFlags() const { return m_updateFlags; }

		/// @brief Sets the update flags for the engine.
		/// @param p_flags The update flags to set.
		void SetUpdateFlags(UpdateFlags p_flags) { m_updateFlags = p_flags; }

		/// @brief Sets or clears a specific update flag bit.
		/// @param p_flag The update flag bit to set or clear.
		/// @param p_value True to set the flag, false to clear it.
		void SetUpdateFlagBits(UpdateFlags p_flags, bool p_value)
		{
			if (p_value)
			{
				// Set the specified flag bits to 1 using bitwise OR
				m_updateFlags = static_cast<UpdateFlags>(
					static_cast<std::uint32_t>(m_updateFlags) | 
					static_cast<std::uint32_t>(p_flags)
				);
			}
			else
			{
				// Clear the specified flag bits to 0 using bitwise AND NOT
				m_updateFlags = static_cast<UpdateFlags>(
					static_cast<std::uint32_t>(m_updateFlags) & 
					~static_cast<std::uint32_t>(p_flags)
				);
			}
		}
		
	private:
		/// @brief The renderer instance, should only be one.
		Graphics::Renderer m_renderer;

		/// @brief The scenemanager instance, should only be one.
		Scene::SceneManager m_sceneManager;

		/// @brief 
		ImGui_ImplVulkan_InitInfo m_initInfo;

		/// @brief A list of event listeners that will be called when an SDL event is polled.
		std::vector<EventListener> m_eventListeners;

		UpdateFlags m_updateFlags{ UpdateFlags::DefaultFlags };
	};
}
