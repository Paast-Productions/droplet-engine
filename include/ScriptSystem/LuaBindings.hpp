#pragma once

#define SOL_ALL_SAFETIES_ON 1

#include "LuaStateHandler.hpp"
#include "LuaApiGenerator.hpp"

#include <sol/sol.hpp>

namespace Droplet::Script
{
	/// @brief Registers engine functionality and types for use in Lua.
	/// LuaBindings provides the interface between the C++ engine and the Lua
	/// scripting environment. It is responsible for registering engine types,
	/// functions, and other functionality that should be accessible from Lua.
	class LuaBindings
	{
	public:
		/// @brief Registers all available engine bindings with Lua.
		/// This function serves as the main entry point for registering C++
		/// functionality that should be exposed to the Lua scripting environment.
		/// @param p_luaState Lua state in which the engine bindings should be
		/// registered.
		static void RegisterBindings(sol::state_view p_luaState);

	private:
		/// @brief Register all global functions for Lua.
		/// Creates OnStart and OnUpdate for lua api
		/// @param p_luaState Lua state in which should register 
		static void RegisterGlobalFunctions();
		/// @brief Registers the Node type with Lua.
		/// Exposes the functionality of Node that is intended to be accessible from Lua scripts.
		/// @param p_luaState Lua state in which the Node bindings should be registered.
		static void RegisterNode(sol::state_view p_luaState);
		/// @brief Registers the Transform
		/// Exposes the functionality of Transform that is intended to be accessible from Lua scripts.
		/// @param p_luaState Lua state in which the Node bindings should be registered.
		static void RegisterTransform(sol::state_view p_luaState);
		/// @brief Registers the GLM type with Lua.
		/// Exposes the functionality of GLM that is intended to be accessible from Lua scripts.
		/// @param p_luaState Lua state in which the Node bindings should be registered.
		static void RegisterGLM(sol::state_view p_luaState);
		/// @brief Registers the GameInput type with Lua.
		/// Exposes the functionality of GameInput that is intended to be accessible from Lua scripts.
		/// @param p_luaState Lua state in which the Node bindings should be registered.
		static void RegisterInput(sol::state_view p_luaState);

		/// @brief Registers the ImGui type with Lua.
		/// Exposes the functionality of ImGui that is intended to be accessible from Lua scripts.
		/// @param p_luaState Lua state in which the ImGui bindings should be registered.
		static void RegisterImGui(sol::state_view p_luaState);

		// TODO: Implement additional engine bindings as functionality is added.

	private:
		/// @brief vector of all GlobalFunctionDefinition for .d.lua file
		static std::vector<LuaGlobalFunctionDefinition> m_luaGlobalDefinitions;
		/// @brief vector of all LuaClassDefinition for .d.lua file
		static std::vector<LuaClassDefinition> m_luaClassDefinitions;
	};
}
