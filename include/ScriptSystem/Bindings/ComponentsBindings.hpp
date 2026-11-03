#pragma once

#include <sol/sol.hpp>

namespace Droplet::Script::Bindings
{
	void RegisterComponents(sol::state_view p_luaState);
	void RegisterCameraComponent(sol::state_view p_luaState);
	void RegisterHealthComponent(sol::state_view p_luaState);
	void RegisterPlayerComponent(sol::state_view p_luaState);
	void RegisterPlayerBehaviour(sol::state_view p_luaState);
}