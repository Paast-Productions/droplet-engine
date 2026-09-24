//#include <gtest/gtest.h>
//#include <ScriptSystem/ScriptSystem.hpp>
//#include <string>
//
//TEST(ScriptSystem, LoadScript)
//{
//	ScriptSystem system;
//	system.SetScriptPath("../src/TestScripts");
//	EXPECT_TRUE(system.LoadScript("testScript.lua"));
//}
//
//TEST(ScriptSystem, UnloadScript)
//{
//	ScriptSystem system;
//	system.SetScriptPath("../src/TestScripts");
//	system.LoadScript("testScript.lua");
//	EXPECT_TRUE(system.UnloadScript("testScript.lua"));
//}
//
//TEST(ScriptSystem, CreateScript)
//{
//	ScriptSystem system;
//	system.SetScriptPath("../src/TestScripts");
//	TestNode testNode;
//
//	bool result = system.CreateScript(&testNode, "testScript.lua");
//	EXPECT_TRUE(result);
//}
//
//TEST(ScriptSystem, DestroyScript)
//{
//	ScriptSystem system;
//	system.SetScriptPath("../src/TestScripts");
//	TestNode firstNode;
//	TestNode secondNode;
//	TestNode thirdNode;
//
//	system.CreateScript(&firstNode, "testScript.lua");
//	system.CreateScript(&secondNode, "testScript.lua");
//	system.CreateScript(&thirdNode, "testScript2.lua");
//
//	system.ActivateScript(&firstNode);
//	system.ActivateScript(&secondNode);
//	system.ActivateScript(&thirdNode);
//
//	system.DetachAllInstancesToScript("testScript.lua");
//
//	EXPECT_FALSE(system.Call(&firstNode, "test").valid());
//	EXPECT_FALSE(system.Call(&secondNode, "test").valid());
//
//	EXPECT_TRUE(system.Call(&thirdNode, "test").valid());
//
//	EXPECT_NO_THROW(system.Update(0.016f));
//
//	EXPECT_NO_THROW(system.DetachAllInstancesToScript("testScript.lua"));
//}
//
//TEST(ScriptSystem, ActivateScript)
//{
//	ScriptSystem system;
//	system.SetScriptPath("../src/TestScripts");
//	TestNode testNode;
//
//	system.CreateScript(&testNode, "testScript3.lua");
//	system.ActivateScript(&testNode);
//	system.Update(1);
//	float result = system.Call(&testNode, "GetDT");
//
//	EXPECT_EQ(result, 1);
//}
//
//TEST(ScriptSystem, DeactivateScript)
//{
//	ScriptSystem system;
//	system.SetScriptPath("../src/TestScripts");
//	TestNode testNode;
//
//	system.CreateScript(&testNode, "testScript3.lua");
//	system.ActivateScript(&testNode);
//	system.Update(1);
//	system.DeactivateScript(&testNode);
//	system.Update(1);
//	auto result = system.Call(&testNode, "GetDT");
//
//	EXPECT_FALSE(result.valid());
//}
