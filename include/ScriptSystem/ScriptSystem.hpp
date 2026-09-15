#pragma once
#include "ScriptManager.hpp"
#include "LuaStateHandler.hpp"
class ScriptSystem
{
public:
	void Initiliaze();
	void Shutdown();
	void Update(float deltaTime);
	void SetScriptDirectory(char* filePath);

private:
	ScriptManager m_scriptManager;
	

};