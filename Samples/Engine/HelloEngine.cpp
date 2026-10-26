#include <Engine/Engine.hpp>
#include <iostream>

// do something snippet example
// [do-something]
// This is written directly in the C++ file, see HelloEngine.cpp
int DoSomething()
{
    return 1 + 1;
}
// [do-something]

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    Droplet::Engine engine({
        Droplet::Graphics::SDL::WindowConfig{
            640, 480, {}
        }
    });


    /*while (engine.Update() == Droplet::DROPLET_RETURNTYPE::OK)
    {
        
    }*/
}
