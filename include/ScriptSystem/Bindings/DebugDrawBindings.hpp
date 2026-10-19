#pragma once

#include <sol/sol.hpp>

namespace Droplet::Script::Bindings
{
	void RegisterDebugDraw(sol::state_view p_luaState);
}