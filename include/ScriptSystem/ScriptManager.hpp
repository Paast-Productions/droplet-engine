#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include "LuaStateHandler.hpp"
#include "TestNode.hpp"
#include "ScriptInstance.hpp"

// This is the script manager
// Its purpose is to manage, handle and load the scripts in the game
// Functonality is to load scripts, see what scripts are loaded, and fetch scripts for the script system and any other system that might need it
class ScriptManager
{
public:

	ScriptManager(LuaStateHandler& p_luaState);
	~ScriptManager() = default;

	void Start();
	void Update(float p_deltaTime);

	ScriptInstance* CreateScript(TestNode* p_testNode, const std::string& p_scriptFile); // add entity as parameter when we have entitites
	void DestroyScript(ScriptInstance* p_scriptInstance);
	bool LoadScript(const std::string& p_scriptFile);
	bool UnloadScript(const std::string& p_scriptFile);
	bool IsLoaded(const std::string& p_scriptFile);
	bool ReloadScript(const std::string& p_scriptFile);
	
	sol::load_result* GetLoadedScript(const std::string& p_scriptFile);

	
private:
	LuaStateHandler& m_StateHandler;
	std::vector<std::unique_ptr<ScriptInstance>> m_scriptInstances;
	std::unordered_map<std::string, sol::load_result> m_loadedScripts;

	bool m_Initialize(); 
	void m_Shutdown();

	bool m_LoadFile(const std::string& p_scriptFile);

	bool m_HandleError(const std::string& p_scriptFile, const sol::error& p_error);

	/// @brief This function finds the path to any script by giving it the name of the script
	/// @param p_scriptFile the name of the scriptfile
	/// @return the path to the scriptfile
	std::filesystem::path FindScript(const std::string& p_scriptFile);
	
};