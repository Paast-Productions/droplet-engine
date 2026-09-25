#include <gtest/gtest.h>
#include <ScriptSystem/ScriptInstance.hpp>
#include <ScriptSystem/LuaBindings.hpp>
#include <SceneSystem/Components/ScriptComponent.hpp>
 
 using namespace Droplet::Script;
 using namespace Droplet::Scene;

TEST(ScriptInstance, getScriptPath)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	ScriptComponent testComp("");

	auto script = stateHandler.GetState().load(R"(
		function OnStart()
		end

		function OnUpdate()
		end
	)");

	ASSERT_TRUE(script.valid());

	ScriptInstance instance(&testComp, stateHandler, script, "TestScript.lua");

	EXPECT_EQ(instance.GetScriptPath(), "TestScript.lua");
}

TEST(ScriptInstance, OnStart)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	LuaBindings::RegisterBindings(stateHandler.GetState());
	ScriptComponent testComp("");

	auto script = stateHandler.GetState().load(R"(
		local test = 0

		function OnStart()
			test = test + 1
		end

		function OnUpdate(dt)
		end

		function getTestValue()
			return test
		end
	)");

	ASSERT_TRUE(script.valid());

	ScriptInstance instance(
		&testComp,
		stateHandler, 
		script, 
		"TestScript.lua"
	);

	instance.OnStart();
	int result = instance.Call("getTestValue");

	EXPECT_EQ(result, 1);
}

TEST(ScriptInstance, OnStartUndefined)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	LuaBindings::RegisterBindings(stateHandler.GetState());
	ScriptComponent testComp("");

	auto script = stateHandler.GetState().load(R"(
		local test = 0

		function onUpdate(dt)
		end

		function getTestValue()
			return test
		end
	)");

	ASSERT_TRUE(script.valid());

	ScriptInstance instance(
		&testComp,
		stateHandler,
		script,
		"TestScript.lua"
	);

	instance.OnStart();
	int result = instance.Call("getTestValue");

	EXPECT_EQ(result, 0);
}

TEST(ScriptInstance, onUpdate)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	LuaBindings::RegisterBindings(stateHandler.GetState());
	ScriptComponent testComp("");

	auto script = stateHandler.GetState().load(R"(
		local test = 0

		function OnStart()
			print("You should not read this!")
		end

		function OnUpdate(dt)
			test = test + dt
		end

		function getTestValue()
			return test
		end
	)");

	ASSERT_TRUE(script.valid());

	ScriptInstance instance(
		&testComp,
		stateHandler,
		script,
		"TestScript.lua"
	);

	for (int i = 0; i < 3; ++i)
	{
		instance.OnUpdate(2.0f);
	}

	float result = instance.Call("getTestValue");

	EXPECT_EQ(result, 6);
}

TEST(ScriptInstance, OnUpdateUndefined)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	LuaBindings::RegisterBindings(stateHandler.GetState());
	ScriptComponent testComp("");

	auto script = stateHandler.GetState().load(R"(
		local test = 0

		function OnStart()
			print("You should not read this!")
		end

		function getTestValue()
			return test
		end
	)");

	ASSERT_TRUE(script.valid());

	ScriptInstance instance(
		&testComp,
		stateHandler,
		script,
		"TestScript.lua"
	);

	for (int i = 0; i < 3; ++i)
	{
		instance.OnUpdate(2.0f);
	}

	int result = instance.Call("getTestValue");

	EXPECT_EQ(result, 0);
}

TEST(ScriptInstance, Call)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	LuaBindings::RegisterBindings(stateHandler.GetState());
	ScriptComponent testComp("");

	auto script = stateHandler.GetState().load(R"(
		local test = 0

		function OnStart()
			print("You should not read this!")
		end

		function OnUpdate(dt)
		end

		function getTestValue()
			return test
		end

		function customFunction(a)
			test = test + a
		end
	)");

	ASSERT_TRUE(script.valid());

	ScriptInstance instance(
		&testComp,
		stateHandler,
		script,
		"TestScript.lua"
	);

	auto result = instance.Call("customFunction", 4);
	int customResult = instance.Call("getTestValue");

	ASSERT_TRUE(result.valid());
	EXPECT_EQ(customResult, 4);
}

TEST(ScriptInstance, CallUndefinedFunction)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	LuaBindings::RegisterBindings(stateHandler.GetState());
	ScriptComponent testComp("");

	auto script = stateHandler.GetState().load(R"(
		function OnStart()
		end

		function OnUpdate(dt)
		end
	)");

	ASSERT_TRUE(script.valid());

	ScriptInstance instance(
		&testComp,
		stateHandler,
		script,
		"TestScript.lua"
	);

	auto result = instance.Call("IDontExist", 4);

	ASSERT_FALSE(result.valid());
}

TEST(ScriptInstance, callTooManyArguments)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	LuaBindings::RegisterBindings(stateHandler.GetState());
	ScriptComponent testComp("");

	auto script = stateHandler.GetState().load(R"(
		local test = 0

		function OnStart()
		end

		function OnUpdate(dt)
		end
		
		function doStuff(a, b)
			test = test + a + b		
		end

		function getTestValue()
			return test
		end
	)");

	ASSERT_TRUE(script.valid());

	ScriptInstance instance(
		&testComp,
		stateHandler,
		script,
		"TestScript.lua"
	);

	auto result = instance.Call("doStuff", 1, 2, 3);
	int doStuffResult = instance.Call("getTestValue");

	ASSERT_TRUE(result.valid());
	EXPECT_EQ(doStuffResult, 3);
}

TEST(ScriptInstance, callTooFewArguments)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	LuaBindings::RegisterBindings(stateHandler.GetState());
	ScriptComponent testComp("");

	auto script = stateHandler.GetState().load(R"(
		local test = 0

		function OnStart()
		end

		function OnUpdate(dt)
		end
		
		function doStuff(a, b)
			test = test + a + b		
		end

		function getTestValue()
			return test
		end
	)");

	ASSERT_TRUE(script.valid());

	ScriptInstance instance(
		&testComp,
		stateHandler,
		script,
		"TestScript.lua"
	);

	auto result = instance.Call("doStuff", 1);
	int doStuffResult = instance.Call("getTestValue");

	ASSERT_FALSE(result.valid());
	EXPECT_EQ(doStuffResult, 0);
}

TEST(ScriptInstance, callError)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	LuaBindings::RegisterBindings(stateHandler.GetState());
	ScriptComponent testComp("");

	auto script = stateHandler.GetState().load(R"(
		function OnStart()
		end

		function OnUpdate(dt)
		end
		
		function add(a, b)
			return a + b		
		end
	)");

	ASSERT_TRUE(script.valid());

	ScriptInstance instance(
		&testComp,
		stateHandler,
		script,
		"TestScript.lua"
	);

	auto result = instance.Call("add", 1, "hello");

	EXPECT_FALSE(result.valid());
}
