#include <Engine/Engine.hpp>

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
	Droplet::Engine engine {};

	while (engine.Update() == Droplet::DROPLET_RETURNTYPE::OK)
	{
		
	}
}