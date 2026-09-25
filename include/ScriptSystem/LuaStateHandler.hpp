#pragma once
#define SOL_ALL_SAFETIES_ON 1

#include <sol/sol.hpp>

namespace Droplet::Script
{

	/// @brief Manages the Lua state used by the scripting system.
	/// LuaStateHandler owns the underlying sol::state and provides access to it
	/// for the other scripting components that need to interact with Lua.
	class LuaStateHandler
	{
	public:
		/// @brief Creates and initializes the Lua state.
		LuaStateHandler();

		/// @brief Destroys the Lua state.
		~LuaStateHandler() = default;

		/// @brief Gets the underlying Lua state.
		/// @return Reference to the managed Lua state.
		sol::state &GetState();

	private:
		/// @brief The Lua state managed by this handler.
		sol::state m_luaState;
	};
}
