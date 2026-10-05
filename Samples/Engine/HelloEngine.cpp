#include <Engine/Engine.hpp>
#include <iostream>

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
	Droplet::Engine engine({
		Droplet::Graphics::SDL::WindowConfig {
			640, 480, {}
		}
	});

	while (engine.Update() == Droplet::DROPLET_RETURNTYPE::OK)
	{
		
	}
}