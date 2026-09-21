#pragma once

#include "ScriptManager.hpp"
#include "LuaStateHandler.hpp"
#include "LuaBindings.hpp"

class ScriptSystem
{
public:
	ScriptSystem();
	~ScriptSystem() = default;

	void Start();
	void Update(float p_deltaTime);

	bool LoadScript(const std::string& p_scriptFile);
	bool UnloadScript(const std::string& p_scriptFile);

	void ActivateScript(TestNode* p_scriptComponent);
	void DeactivateScript(TestNode* p_scriptComponent);

	ScriptInstance* CreateScript(TestNode* testNode, const std::string& p_scriptFile);
	void DestroyScript(ScriptInstance* p_scriptInstance);

	template<typename... Args>
	sol::protected_function_result Call(TestNode* p_scriptComponent, std::string_view p_funcitonName, Args&&... p_args);

	//void SetScriptDirectory(char* filePath);

private:
	LuaStateHandler m_luaStateHandler;
	ScriptManager m_scriptManager;
};

template<typename... Args>
inline sol::protected_function_result ScriptSystem::Call(TestNode* p_scriptComponent, std::string_view p_functionName, Args&&... p_args)
{
	return m_scriptManager.Call(p_scriptComponent, p_functionName, std::forward<Args>(p_args)...);
}