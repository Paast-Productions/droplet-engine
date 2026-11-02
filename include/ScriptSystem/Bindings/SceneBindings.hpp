#pragma once

#include "SceneManager.hpp"
#include <sol/sol.hpp>

namespace Droplet::Script::Bindings
{
	void RegisterScenes(sol::state_view p_luaState, Droplet::Scene::SceneManager &p_sceneManager);
}