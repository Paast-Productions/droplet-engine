#include "Graphics/VK/Renderer.hpp"

#include <print>
#include <filesystem>
#include <array>
#include <string>
#include <slang/slang.h>
#include <slang/slang-com-ptr.h>

[[nodiscard]] Slang::ComPtr<slang::IBlob> CompileShader();

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	Slang::ComPtr<slang::IBlob> shaderBlob { CompileShader() };
	
    std::print("Hello Triangle!\n");

	bool done = false;


	Renderer rnd;

	if (rnd.Initialize(shaderBlob) == 1)
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

Slang::ComPtr<slang::IBlob> CompileShader()
{
	Slang::ComPtr<slang::IGlobalSession> globalSession {};
	slang::createGlobalSession(globalSession.writeRef());
	
	slang::TargetDesc targetDesc 
	{
		.format = SLANG_SPIRV,
		.profile = globalSession->findProfile("spirv_1_4")
	};
	
	std::array<slang::PreprocessorMacroDesc, 2> preprocessorMacroDesc
	{
		{
			{ "BIAS_VALUE", "1138" },
			{ "OTHER_MACRO", "float" }
		}
	};
	
	std::array<slang::CompilerOptionEntry, 1> options
	{
		{
			{
				.name = slang::CompilerOptionName::EmitSpirvDirectly,
				.value = 
				{
					slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr
				}
			}	
		}
	};
	
	slang::SessionDesc sessionDesc 
	{
		.targets = &targetDesc,
		.targetCount = 1,
		.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
		.preprocessorMacros = preprocessorMacroDesc.data(),
		.preprocessorMacroCount = preprocessorMacroDesc.size(),
		.compilerOptionEntries = options.data(),
		.compilerOptionEntryCount = options.size()
	};
	
	Slang::ComPtr<slang::ISession> session {};
	globalSession->createSession(sessionDesc, session.writeRef());
	
	// LOAD SHADER MODULE
	Slang::ComPtr<slang::IModule> module {};
	
	{
		Slang::ComPtr<slang::IBlob> diagnosticsBlob {};
		std::string moduleName {"shader"};
		module = session->loadModule(moduleName.c_str(),  diagnosticsBlob.writeRef());
		
		if (!module)
		{
			std::print("Diagnostics: {0}", static_cast<const char*>(diagnosticsBlob->getBufferPointer()));
		}
	}
	
	// ENTRY POINT
	Slang::ComPtr<slang::IEntryPoint> entryPoint {};
	
	{
		Slang::ComPtr<slang::IBlob> diagnosticsBlob {};
		module->findEntryPointByName("vertMain", entryPoint.writeRef());
		
		if (!entryPoint)
		{
			std::print("Diagnostics: {0}", static_cast<const char*>(diagnosticsBlob->getBufferPointer()));
		}
	}
	
	// COMPOSE PROGRAM
	std::array<slang::IComponentType*, 1> componentTypes 
	{
		{
			module
		}
	};
	
	Slang::ComPtr<slang::IComponentType> composedProgram {};
	
	{
		Slang::ComPtr<slang::IBlob> diagnosticsBlob {};
		SlangResult result 
		{
			session->createCompositeComponentType
			(
				componentTypes.data(),
				componentTypes.size(),
				composedProgram.writeRef(),
				diagnosticsBlob.writeRef()
			)
		};
		
		if (result == SLANG_FAIL)
		{
			std::print("Diagnostics: {0}", static_cast<const char*>(diagnosticsBlob->getBufferPointer()));
			throw std::runtime_error("You fucked up");
		}
	}
	
	// LINK
	Slang::ComPtr<slang::IComponentType> linkedProgram {};
	
	{
		Slang::ComPtr<slang::IBlob> diagnosticsBlob {};
		SlangResult result
		{
			composedProgram->link
			(
				linkedProgram.writeRef(),
				diagnosticsBlob.writeRef()
			)
		};
		
		if (result == SLANG_FAIL)
		{
			std::print("Diagnostics: {0}", static_cast<const char*>(diagnosticsBlob->getBufferPointer()));
			throw std::runtime_error("You fucked up");
		}
	}
	
	// COMPILE
	Slang::ComPtr<slang::IBlob> spirvCode {};
	
	{
		Slang::ComPtr<slang::IBlob> diagnosticsBlob {};
		SlangResult result 
		{
			linkedProgram->getTargetCode
			(
				0,
				spirvCode.writeRef(),
				diagnosticsBlob.writeRef()
			)
		};
		
		if (result == SLANG_FAIL)
		{
			std::print("Diagnostics: {0}", static_cast<const char*>(diagnosticsBlob->getBufferPointer()));
			throw std::runtime_error("You fucked up");
		}
	}
	
	return spirvCode;
}
