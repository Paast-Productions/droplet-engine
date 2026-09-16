#pragma once

#include <string>
#include "LuaStateHandler.hpp"
#include "ScriptInstance.hpp"
#include <vector>

// This is the script manager
// Its purpose is to manage, handle and load the scripts in the game
// Functonality is to load scripts, see what scripts are loaded, and fetch scripts for the script system and any other system that might need it
class ScriptManager
{
public:

	ScriptManager(LuaStateHandler& p_statehandler);
	~ScriptManager() = default;

	void Update(float p_deltaTime);
	void Shutdown();

	ScriptInstance* CreateScript(const std::string& p_scriptFile); // add entity as parameter when we have entitites
	void DestroyScript(const std::string& p_scriptFile);

	bool LoadScript(const std::string& p_scriptFile);
	void UnloadScript(const std::string& p_scriptFile);
	bool IsLoaded(const std::string& p_scriptFile);
	bool ReloadScript(const std::string& scriptFile);
	
	sol::table& getScript(const std::string& p_scriptFile);

	
private:
	LuaStateHandler& m_StateHandler;

	std::vector<std::unique_ptr<ScriptInstance>> m_scripts;

	bool m_Initialize();
	void m_Shutdown();

	bool m_LoadFile(const std::string& p_scriptFile);

	bool m_handleError(const std::string& p_scriptFile, const sol::error& p_error);


	
};