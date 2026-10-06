#pragma once

#include <Graphics/VK/Renderer.hpp>
#include <Graphics/SDL/Window.hpp>
#include <Graphics/SDL/Event.hpp>
#include <ScriptSystem/ScriptSystem.hpp>
#include <Time.hpp>

namespace Droplet
{
	enum class DROPLET_RETURNTYPE
	{
		OK,
		EXIT,
		EXIT_ERROR
	};
	
	struct EngineConfig
	{
		Graphics::SDL::WindowConfig WindowConfig;
	};
	
	/// @brief Engine owns all long-lived engine subsystems. The engine is responsible for initializing the subsystems and to destroy them.
	/// @details The engine class should not be bloated with various different functions from subsystems, rather it should give access to the 
	/// subsystems via some getter function. The engine class is responsible for updating subsystems.
	class Engine
	{
	public:
		Engine(EngineConfig p_config);
		
		/// @brief Will update the subsystems. Is needed to run anything.
		/// @return The droplet returntype can be used to detemine if the update was successful or not.
		[[nodiscard]] DROPLET_RETURNTYPE Update();
		
		/// @brief the purpose of GetWindow is to let other systems use the window. 
		/// Mainly the editor and the game itself needs access to the window.
		/// @return An SDL window
		[[nodiscard]] SDL_Window *GetWindow();
		
	private:
		/// @brief The renderer instance, should only be one.
		Graphics::Renderer m_renderer;
		Graphics::SDL::Event m_event {};
	};
}
