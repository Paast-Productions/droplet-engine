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

	void CreateScript(TestNode* testNode, const std::string& p_scriptFile);
	void DestroyScript(const std::string& p_scriptFile);

	//void SetScriptDirectory(char* filePath);

private:
	LuaStateHandler m_luaStateHandler;
	ScriptManager m_scriptManager;
};