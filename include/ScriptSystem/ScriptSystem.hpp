#pragma once

#include "ScriptManager.hpp"
#include "LuaStateHandler.hpp"

#include <SceneSystem/Component.hpp>
#include <SceneSystem/Behaviour.hpp>

namespace Droplet::Script
{
	/// @brief Provides the main interface for the engine's Lua scripting system.
	/// ScriptSystem manages the lifecycle of scripts and coordinates between the
	/// Lua state and the ScriptManager. It is responsible for starting and
	/// updating the scripting system, loading and unloading script files, and
	/// creating, activating, deactivating, and destroying script instances.
	class ScriptSystem
	{
	public:
		/// @brief Constructs the script system.
		ScriptSystem();

		/// @brief Destroy the script system.
		~ScriptSystem();

		/// @brief Copy constructor deleting any attempt to create a 
		/// new scriptsystem using an existing scriptsystem
		/// @param ScriptSystem
		ScriptSystem(const ScriptSystem &) = delete;

		/// @brief Copy assignment operator deleting itself
		/// @param ScriptSystem
		ScriptSystem &operator=(const ScriptSystem &) = delete;

		[[nodiscard]] static ScriptSystem &Get()
		{
			static ScriptSystem scriptSystem;
			return scriptSystem;
		}

		/// @brief Starts the script script system.
		/// This should be called once the scene starts with the script system.
		void Start();

		/// @brief Updates the scripting system.
		/// This should be called once per frame to allow active scripts to perform
		/// their per-frame update logic.
		/// @param p_deltaTime Time elapsed since the previous update, in seconds.
		void Update(float p_deltaTime);

		/// @brief Creates a script instance for a component.
		/// @param p_scriptComponent Component that will connect to the script instance.
		/// @param p_scriptFile Path to the Lua script file associated with the instance.
		void CreateComponentScript(Droplet::Scene::Component *p_scriptComponent, const std::string &p_scriptFile);

		/// @brief Creates a script instance for a behaviour
		/// @param p_scirptBehaviour Behaviour that will connect to the script instance.
		/// @param p_scriptFile Path to the Lua script file associated with the instance.
		void CreateBehaviourScript(Droplet::Scene::Behaviour *p_scriptBehaviour, const std::string &p_scriptFile);

		/// @brief Loads a Lua script from a file.
		/// @param p_scriptFile Path to the Lua script file.
		void LoadScript(const std::string &p_scriptFile);

		/// @brief Unloads a previously loaded Lua script.
		/// @param p_scriptFile Path to the Lua script file.
		void UnloadScript(const std::string &p_scriptFile);

		/// @brief Activates a script component.
		/// An activated script component is allowed to participate in the scripting
		/// system's update and execution flow.
		/// @param p_scriptComponent Script component to activate.
		void ActivateComponentScript(Droplet::Scene::Component *p_scriptComponent);

		/// @brief Activates a script component.
		/// An activated script behaviour is allowed to participate in the scripting
		/// system's update and execution flow.
		/// @param p_scriptBehaviour Script behaviour to activate.
		void ActivateBehaviourScript(Droplet::Scene::Behaviour *p_scriptBehaviour);

		/// @brief Deactivates a script behaviour.
		/// A deactivated script component will no longer participate in the
		/// scripting system's update and execution flow, that involves OnStart, OnUpdate, and any Call function you do.
		/// @param p_scriptComponent Script component to deactivate.
		void DeactivateComponentScript(Droplet::Scene::Component *p_scriptComponent);

		/// @brief Deactivates a script behaviour.
		/// A deactivated script component will no longer participate in the
		/// scripting system's update and execution flow, that involves OnStart, OnUpdate, and any Call function you do.
		/// @param p_scriptBehaviour Script component to deactivate.
		void DeactivateBehaviourScript(Droplet::Scene::Behaviour *p_scriptBehaviour);
		
		/// @brief Detach Script from Component
		/// @param p_scriptComponent 
		void DetachComponentScript(Droplet::Scene::Component *p_scriptComponent);

		/// @brief Detach Script from Behaviour
		/// @param p_scriptBehaviour
		void DetachBehaviourScript(Droplet::Scene::Behaviour *p_scriptBehaviour);

		/// @brief Detaches components to a certain script
		/// @param p_scriptInstance This is the lua file you want to disconnect all instances to
		void DetachAllInstancesToScript(const std::string &p_scriptPath);
		
		/// @brief Set path for where the scripts is
		/// @param p_directoryPath
		void SetScriptPath(const std::string &p_directoryPath);

		/// @brief Calls a Lua function on a script component.
		/// The supplied arguments are forwarded to the Lua function. The result
		/// contains the status and return values of the protected Lua function call.
		/// @tparam Args Types of the arguments passed to the Lua function.
		/// @param p_scriptComponent Script component containing the function to call.
		/// @param p_functionName Name of the Lua function to call.
		/// @param p_args Arguments to forward to the Lua function.
		/// @return Result of the protected Lua function call.
		template<typename... Args>
		sol::protected_function_result Call(
			Droplet::Scene::Component *p_scriptComponent, const std::string &p_functionName, Args&&... p_args);

	private:
		/// @brief Handles the Lua state used by the scripting system.
		LuaStateHandler m_luaStateHandler;

		/// @brief Manages script instances and their interaction with Lua.
		ScriptManager m_scriptManager;
	};

	/// @brief Calls a Lua function through the ScriptManager.
	/// @tparam Args Types of the arguments passed to the Lua function.
	/// @param p_scriptComponent Script component containing the function to call.
	/// @param p_functionName Name of the Lua function to call.
	/// @param p_args Arguments to forward to the Lua function.
	/// @return Result of the protected Lua function call.
	template<typename... Args>
	inline sol::protected_function_result ScriptSystem::Call(
		Droplet::Scene::Component *p_scriptComponent, const std::string &p_functionName, Args&&... p_args)
	{
		return m_scriptManager.Call(p_scriptComponent, p_functionName, std::forward<Args>(p_args)...);
	}
}
