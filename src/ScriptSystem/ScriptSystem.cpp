#include "ScriptSystem.hpp"

void ScriptSystem::Initiliaze()
{

}

void ScriptSystem::Shutdown()
{
}

void ScriptSystem::Update(float deltaTime)
{
	m_scriptManager.Update(deltaTime);
}
