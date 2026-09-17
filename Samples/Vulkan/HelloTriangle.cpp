#include <print>
#include <filesystem>
#include <array>
#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#include "Renderer.hpp"

void CompileShader(std::filesystem::path p_path);

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    std::print("Hello Triangle!\n");

	bool done = false;


	Renderer rnd;

	if (rnd.Initialize() == 1)
	{
		return 1;
	}

	while (!done)
	{
		while (SDL_PollEvent(&rnd.p_event))
		{
			if (rnd.p_event.type == SDL_EVENT_QUIT)
			{
				done = true;
			}

			if (rnd.p_event.type == SDL_EVENT_WINDOW_RESIZED || rnd.p_event.type == SDL_EVENT_WINDOW_MINIMIZED)
			{
				rnd.windowResize();
			}

			rnd.drawFrame();

			if (rnd.p_event.type == SDL_EVENT_KEY_DOWN) {
				if (rnd.p_event.key.key == SDLK_ESCAPE) {
					done = true;
				}
			}
		}
	}
    
    return 0;
}

void CompileShader(std::filesystem::path p_path)
{
	Slang::ComPtr<slang::IGlobalSession> globalSession {};
	slang::createGlobalSession(globalSession.writeRef());
	
	slang::TargetDesc targetDesc 
	{
		.format = SLANG_SPIRV,
		.profile = globalSession->findProfile("spirv_1_5")
	};
	
	std::array<slang::PreprocessorMacroDesc, 2> preprocessorMacroDescs
	{
		{
			{ "BIAS_VALUE", "1138" },
			{ "OTHER_MACRO", "float" }
		}
	};
	
	slang::SessionDesc sessionDesc 
	{
		.targets = &targetDesc,
		.targetCount = 1,
		.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR
	};
	
	Slang::ComPtr<slang::ISession> session {};
	globalSession->createSession(sessionDesc, session.writeRef());
	
	Slang::ComPtr<slang::IBlob> diagnostics {};
	Slang::ComPtr<slang::IModule> module
	(
		session->loadModule("shader", diagnostics.writeRef())
	);
	
	if (!module)
	{
		std::print("Diagnostics: {0}", static_cast<const char*>(diagnostics->getBufferPointer()));
	}
}
