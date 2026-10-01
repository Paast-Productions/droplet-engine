#pragma once

#include <Graphics/VK/Renderer.hpp>
#include <Graphics/SDL/Window.hpp>

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
		
		[[nodiscard]] DROPLET_RETURNTYPE Update();
		
		[[nodiscard]] Graphics::SDL::Window &GetWindow();
		
	private:
		Renderer m_renderer;
	};
}
