#include <gtest/gtest.h>
#include <ScriptSystem/ScriptInstance.hpp>
#include <ScriptSystem/LuaBindings.hpp>

TEST(ScriptInstance, getScriptPath)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	TestNode testNode;

	auto script = stateHandler.GetState().load(R"(
		function onStart()
		end

		function onUpdate()
		end
	)");

	ASSERT_TRUE(script.valid());

	ScriptInstance instance(&testNode, stateHandler, script, "TestScript.lua");

	EXPECT_EQ(instance.getScriptPath(), "TestScript.lua");
}

TEST(ScriptInstance, onStart)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	LuaBindings::RegisterBindings(stateHandler.GetState());
	TestNode testNode;
	testNode.setPosition(0, 0, 0);

	auto script = stateHandler.GetState().load(R"(
		function onStart()
			self:set_position(
				self:get_x() + 1.0,
				self:get_y(),
				self:get_z()
			)
		end

		function onUpdate(dt)
		end
	)");

	ASSERT_TRUE(script.valid());

	ScriptInstance instance(
		&testNode, 
		stateHandler, 
		script, 
		"TestScript.lua"
	);

	instance.onStart();

	EXPECT_EQ(testNode.getX(), 1);
}

TEST(ScriptInstance, onStartUndefined)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	LuaBindings::RegisterBindings(stateHandler.GetState());
	TestNode testNode;
	testNode.setPosition(1, 0, 0);

	auto script = stateHandler.GetState().load(R"(
		function onUpdate(dt)
		end
	)");

	ASSERT_TRUE(script.valid());

	ScriptInstance instance(
		&testNode,
		stateHandler,
		script,
		"TestScript.lua"
	);

	instance.onStart();

	EXPECT_EQ(testNode.getX(), 1);
}

TEST(ScriptInstance, onUpdate)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	LuaBindings::RegisterBindings(stateHandler.GetState());
	TestNode testNode;
	testNode.setPosition(0, 0, 0);

	auto script = stateHandler.GetState().load(R"(
		function onStart()
			print("You should not read this!")
		end

		function onUpdate(dt)
			self:set_position(
				self:get_x(),
				self:get_y() + dt,
				self:get_z()
			)
		end
	)");

	ASSERT_TRUE(script.valid());

	ScriptInstance instance(
		&testNode,
		stateHandler,
		script,
		"TestScript.lua"
	);

	for (int i = 0; i < 3; ++i)
	{
		instance.onUpdate(2.0f);
	}

	EXPECT_EQ(testNode.getY(), 6);
}

TEST(ScriptInstance, onUpdateUndefined)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	LuaBindings::RegisterBindings(stateHandler.GetState());
	TestNode testNode;
	testNode.setPosition(0, 1, 0);

	auto script = stateHandler.GetState().load(R"(
		function onStart()
			print("You should not read this!")
		end
	)");

	ASSERT_TRUE(script.valid());

	ScriptInstance instance(
		&testNode,
		stateHandler,
		script,
		"TestScript.lua"
	);

	for (int i = 0; i < 3; ++i)
	{
		instance.onUpdate(2.0f);
	}

	EXPECT_EQ(testNode.getY(), 1);
}

TEST(ScriptInstance, call)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	LuaBindings::RegisterBindings(stateHandler.GetState());
	TestNode testNode;
	testNode.setPosition(0, 0, 0);

	auto script = stateHandler.GetState().load(R"(
		function onStart()
		end

		function onUpdate(dt)
		end

		function customFunction(a)
			self:set_position(
				self:get_x(),
				self:get_y(),
				self:get_z() + a
			)
		end
	)");

	ASSERT_TRUE(script.valid());

	ScriptInstance instance(
		&testNode,
		stateHandler,
		script,
		"TestScript.lua"
	);

	auto result = instance.call("customFunction", 4);

	ASSERT_TRUE(result.valid());
	EXPECT_EQ(testNode.getZ(), 4);
}

TEST(ScriptInstance, callUndefinedFunction)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	LuaBindings::RegisterBindings(stateHandler.GetState());
	TestNode testNode;
	testNode.setPosition(0, 0, 1);

	auto script = stateHandler.GetState().load(R"(
		function onStart()
		end

		function onUpdate(dt)
		end
	)");

	ASSERT_TRUE(script.valid());

	ScriptInstance instance(
		&testNode,
		stateHandler,
		script,
		"TestScript.lua"
	);

	auto result = instance.call("IDontExist", 4);

	ASSERT_FALSE(result.valid());
	EXPECT_EQ(testNode.getZ(), 1);
}

TEST(ScriptInstance, callTooManyArguments)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	LuaBindings::RegisterBindings(stateHandler.GetState());
	TestNode testNode;
	testNode.setPosition(1, 1, 1);

	auto script = stateHandler.GetState().load(R"(
		function onStart()
		end

		function onUpdate(dt)
		end
		
		function doStuff(a, b)
			self:set_position(
				self:get_x() + a,
				self:get_y() + b,
				self:get_z()
			)			
		end
	)");

	ASSERT_TRUE(script.valid());

	ScriptInstance instance(
		&testNode,
		stateHandler,
		script,
		"TestScript.lua"
	);

	auto result = instance.call("doStuff", 1, 2, 3);

	ASSERT_FALSE(result.valid());
	EXPECT_EQ(testNode.getX(), 1);
	EXPECT_EQ(testNode.getY(), 1);
	EXPECT_EQ(testNode.getZ(), 1);
}

TEST(ScriptInstance, callTooFewArguments)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	LuaBindings::RegisterBindings(stateHandler.GetState());
	TestNode testNode;
	testNode.setPosition(1, 1, 1);

	auto script = stateHandler.GetState().load(R"(
		function onStart()
		end

		function onUpdate(dt)
		end
		
		function doStuff(a, b)
			self:set_position(
				self:get_x() + a,
				self:get_y() + b,
				self:get_z()
			)			
		end
	)");

	ASSERT_TRUE(script.valid());

	ScriptInstance instance(
		&testNode,
		stateHandler,
		script,
		"TestScript.lua"
	);

	auto result = instance.call("doStuff", 1);

	ASSERT_FALSE(result.valid());
	EXPECT_EQ(testNode.getX(), 1);
	EXPECT_EQ(testNode.getY(), 1);
	EXPECT_EQ(testNode.getZ(), 1);
}

//Call function tests list :)
//existing function
//arguments
//return value
//function with no return
//nonexistent function
//Lua function that throws an error