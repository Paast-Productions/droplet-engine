#pragma once

#include <string>
#include "LuaStateHandler.hpp"
#include "ScriptInstance.hpp"

// This is the script manager
// Its purpose is to manage, handle and load the scripts in the game
// Functonality is to load scripts, see what scripts are loaded, and fetch scripts for the script system and any other system that might need it
class ScriptManager
{
public:

	ScriptManager();
	~ScriptManager() = default;

	void Update(float deltaTime);
	void Shutdown();

	ScriptInstance CreateScript();
	void DestroyScript();
	
	bool LoadScript();
	void UnloadScript(const std::string& scriptFile);
	bool IsLoaded(const std::string& scriptFile);
	bool ReloadScript(const std::string& scriptFile);

	sol::table& getScript(const std::string& scriptFile);

	
private:
	LuaStateHandler* m_StateHandler;
	bool m_Initialize();
	void m_Shutdown();

	bool m_LoadFile(const std::string& scriptFile);

	bool m_handleError(const std::string& scriptFile, const sol::error& error);


	
};