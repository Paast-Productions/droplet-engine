#pragma once
#include "ScriptManager.hpp"
#include "LuaStateHandler.hpp"

class ScriptSystem
{
public:
	ScriptSystem() = default;
	~ScriptSystem() = default;

	void Initialize();
	void Update(float deltaTime);
	void Shutdown();

	void CreateScript();
	void DestroyScript();

	//void SetScriptDirectory(char* filePath);

private:
	LuaStateHandler m_luaStateHandler;
	ScriptManager m_scriptManager;
};