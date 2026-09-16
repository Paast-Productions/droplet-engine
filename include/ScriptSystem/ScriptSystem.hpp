#pragma once

#include "ScriptManager.hpp"
#include "LuaStateHandler.hpp"
#include "LuaBindings.hpp"

class ScriptSystem
{
public:
	ScriptSystem();
	~ScriptSystem() = default;

	void Initialize();
	void Update(float deltaTime);
	void Shutdown();

	void CreateScript(const std::string& p_scriptFile);
	void DestroyScript(const std::string& p_scriptFile);

	//void SetScriptDirectory(char* filePath);

private:
	LuaStateHandler m_luaStateHandler;
	ScriptManager m_scriptManager;
};