#pragma once

#include <Graphics/VK/Renderer.hpp>
#include <Graphics/SDL/Window.hpp>
#include <ScriptSystem/ScriptSystem.hpp>

class Time; // Forward declaration ahead of time implementation

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
	
	class Engine
	{
	public:
		Engine(EngineConfig p_config);
		
		void Run();
		void ShutDown();
		
		[[nodiscard]] DROPLET_RETURNTYPE Update();
		
		[[nodiscard]] Graphics::SDL::Window &GetWindow();
		
	private:
		Renderer m_renderer;
		Script::ScriptSystem m_scriptSystem;
	};
}
