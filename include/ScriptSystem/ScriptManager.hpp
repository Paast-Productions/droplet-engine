#pragma once

#include <string>
#include "LuaStateHandler.hpp"
#include "TestNode.hpp"
#include "ScriptInstance.hpp"
#include <vector>

// This is the script manager
// Its purpose is to manage, handle and load the scripts in the game
// Functonality is to load scripts, see what scripts are loaded, and fetch scripts for the script system and any other system that might need it
class ScriptManager
{
public:

	ScriptManager(LuaStateHandler& p_luaState);
	~ScriptManager() = default;

	void Update(float p_deltaTime);
	
	/// <summary>
	/// Takes a component as a parameter and creates a relationships between the component and the desired lua file
	/// </summary>
	/// <param name="p_testNode"></param>
	/// <param name="p_scriptFile"></param>
	/// <returns></returns>
	ScriptInstance* CreateScript(TestNode* p_scriptComponent, const std::string& p_scriptFile); // add entity as parameter when we have entitites


	/// <summary>
	/// Detaches the relation of the instance on a component
	/// </summary>
	/// <param name="p_testNode"></param>
	/// <param name="p_scriptInstance"></param>
	void DetachScript(TestNode* p_scriptComponent);
	/// <summary>
	/// Deletes every instance of a specific script across all relationships
	/// </summary>
	/// <param name="p_scriptInstance"></param>
	void DestroyScript(ScriptInstance* p_scriptInstance);

	bool LoadScript(const std::string& p_scriptFile);
	bool UnloadScript(const std::string& p_scriptFile);
	bool IsLoaded(const std::string& p_scriptFile);
	bool ReloadScript(const std::string& p_scriptFile);
	
	sol::load_result* GetLoadedScript(const std::string& p_scriptFile);

	
private:
	//state
	LuaStateHandler& m_StateHandler;
	//Owner
	std::vector<std::unique_ptr<ScriptInstance>> m_scriptInstances;
	//Component -> Instance relationship
	std::unordered_map< TestNode*, ScriptInstance*> m_scripts;
	//loaded Lua chunks
	std::unordered_map<std::string, sol::load_result> m_loadedScripts;
	bool m_Initialize(); 
	void m_Shutdown();
	void m_DestroyInstance(ScriptInstance* p_scriptInstance)

	bool m_LoadFile(const std::string& p_scriptFile);

	bool m_HandleError(const std::string& p_scriptFile, const sol::error& p_error);


	
};