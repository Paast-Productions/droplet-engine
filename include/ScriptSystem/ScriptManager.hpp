#pragma once

#include <string>
#include "LuaStateHandler.hpp"
#include "TestNode.hpp"
#include "ScriptInstance.hpp"
#include <vector>
#include <filesystem>
// This is the script manager
// Its purpose is to manage, handle and load the scripts in the game
// Functonality is to load scripts, see what scripts are loaded, and fetch scripts for the script system and any other system that might need it
class ScriptManager
{
public:

	ScriptManager(LuaStateHandler& p_luaState);
	~ScriptManager() = default;

	void Update(float p_deltaTime);
	void Start();

	template<typename... Args>
	sol::protected_function_result Call(
		TestNode* p_scriptComponent, std::string_view p_functionName, Args&&... p_args);

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
	/// @brief  Deletes every instance of a specific script across all relationships
	/// @param p_scriptInstance 
	void DestroyScript(ScriptInstance* p_scriptInstance);

	/// <summary>
	/// Loads a script and pts the loaded result into its member variable, noteworthy is that this function returns true in two different scenarios, the script loaded successfully, or the script was already loaded.
	/// </summary>
	/// <param name="p_scriptFile"></param>
	/// <returns></returns>
	bool LoadScript(const std::string& p_scriptFile);
	bool UnloadScript(const std::string& p_scriptFile);
	bool IsLoaded(const std::string& p_scriptFile);
	bool ReloadScript(const std::string& p_scriptFile);
	
	sol::load_result* GetLoadedScript(const std::string& p_scriptFile);
	/// @brief Activates the script and runs on update each frame, This solution is O(n) time complexity, another solution with storing indexes in the instances can make this O(1)
	/// @param p_scriptComponent 
	void ActivateScript(TestNode* p_scriptComponent);
	/// @brief Deactivates a script and no longer runs on update, This solution is O(n) time complexity, another solution with storing indexes in the instances can make this O(1)
	/// @param p_scriptComponent 
	void DeActivateScript(TestNode* p_scriptComponent);
	
private:
	//state
	LuaStateHandler& m_StateHandler;
	//Owner
	std::vector<std::unique_ptr<ScriptInstance>> m_scriptInstances;
	//Component -> Instance relationship
	std::unordered_map< TestNode*, ScriptInstance*> m_scripts;
	//loaded Lua chunks
	std::unordered_map<std::string, sol::load_result> m_loadedScripts;
	//Active  scripts
	std::vector<ScriptInstance*> m_activeScripts;
	bool m_Initialize(); 
	void m_Shutdown();
	void m_DestroyInstance(ScriptInstance* p_scriptInstance);

	bool m_LoadFile(const std::string& p_scriptFile);

	bool m_HandleError(const std::string& p_scriptFile, const sol::error& p_error);

	/// @brief This function finds the path to any script by giving it the name of the script
	/// @param p_scriptFile the name of the scriptfile
	/// @return the path to the scriptfile
	std::filesystem::path m_FindScript(const std::string& p_scriptFile);

	
};

template <typename... Args>
inline sol::protected_function_result ScriptManager::Call(TestNode* p_scriptComponent, std::string_view p_functionName, Args&&... p_args)
{
	if (p_scriptComponent == nullptr)
	{
		// Don't send in nullptt, send error to logging manager
		return {};
	}

	std::unordered_map<TestNode*, ScriptInstance*>::iterator it = m_scripts.find(p_scriptComponent);

	if (it == m_scripts.end())
	{
		//component has no attached script
		return {};
	}

	return it->second->call(p_functionName, std::forward<Args>(p_args)...);
}