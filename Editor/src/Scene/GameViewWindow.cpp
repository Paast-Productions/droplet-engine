#include "GameViewWindow.hpp"

#include <tracy/public/tracy/Tracy.hpp>
#include <EditorConfig.hpp>
#include <Engine/EngineFlags.hpp>
#include <IconFontCppHeaders/IconsLucide.h>
#include <Graphics/DebugDraw.hpp>
#include <Graphics/VK/IntermediaryStorage.hpp>
#include <ImGuiUtils.hpp>

using namespace Droplet;
using namespace Droplet::Editor;
using namespace Droplet::Editor::Scene;
using namespace Droplet::Graphics::Debug;

void GameViewWindow::InitImpl()
{
	SetName("Game View");
	//ToggleToolbar(true);
	
}

void GameViewWindow::RenderToolbar()
{

}

void GameViewWindow::RenderImpl()
{
	// TODO
}
