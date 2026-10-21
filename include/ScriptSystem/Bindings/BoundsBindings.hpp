#pragma once

#include <sol/sol.hpp>

namespace Droplet::Script::Bindings
{
	void RegisterBounds(sol::state_view p_luaState);
}