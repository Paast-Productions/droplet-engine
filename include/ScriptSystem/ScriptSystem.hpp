#pragma once

#include "ScriptManager.hpp"

class ScriptSystem
{
public:
	ScriptSystem();
	~ScriptSystem() = default;

	void Initialize();
	void Update(float p_deltaTime);
	void Shutdown();

	bool LoadScript(const std::string& p_scriptFile);
	bool UnloadScript(const std::string& p_scriptFile);

	ScriptInstance* CreateScript(TestNode* testNode, const std::string& p_scriptFile);
	void DestroyScript(ScriptInstance* p_scriptInstance);

	//void SetScriptDirectory(char* filePath);

private:
	LuaStateHandler m_luaStateHandler;
	ScriptManager m_scriptManager;
};