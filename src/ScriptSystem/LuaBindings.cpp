#include "LuaBindings.hpp"
#include "Node.hpp"
#include "Transform.hpp"
#include <glm/glm.hpp>
#include "GameInput.hpp"

#include <print>
#include <filesystem>

using namespace Droplet;

std::vector<Script::LuaGlobalFunctionDefinition> Script::LuaBindings::m_luaGlobalDefinitions;
std::vector<Script::LuaClassDefinition> Script::LuaBindings::m_luaClassDefinitions;

void Script::LuaBindings::RegisterBindings(sol::state_view p_luaState)
{
    RegisterGlobalFunctions();
    RegisterNode(p_luaState);
    RegisterTransform(p_luaState);
    RegisterGLM(p_luaState);
	RegisterInput(p_luaState);

	std::filesystem::path scriptDirectory = std::filesystem::current_path()
		/ ".." / ".." / ".." / "src" / "TestScripts"; // Not sure if this should be hardcoded like this :)

    std::filesystem::path path = scriptDirectory / "LuaHelper.d.lua";
	LuaApiGenerator::Generate(path, m_luaGlobalDefinitions, m_luaClassDefinitions);
}

void Script::LuaBindings::RegisterGlobalFunctions()
{
    m_luaGlobalDefinitions =
    {
        {
            "OnStart",
            "void",
            {}
        },
        {
            "OnUpdate",
            "void",
            {
                { "dt", "number" }
            } 
        }
    };
}
 
void Script::LuaBindings::RegisterNode(sol::state_view p_luaState)
{
    p_luaState.new_usertype<Scene::Node>(
        "Node",
        "GetName", &Droplet::Scene::Node::GetName,
        "GetTransform", static_cast<Scene::Transform &(Scene::Node::*)()>(&Scene::Node::GetTransform)
    );

    LuaClassDefinition classDef;
    classDef.name = "Node";

    classDef.functions =
    {
        {
            "GetName",
            "string",
            {}
        },
        {
            "GetTransform",
            "Transform",
            {}
        }
    };

    LuaBindings::m_luaClassDefinitions.push_back(classDef);
}

void Script::LuaBindings::RegisterTransform(sol::state_view p_luaState)
{
    p_luaState.new_enum<Scene::Transform::Space>(
        "TransformSpace",
        {
            {"Local", Scene::Transform::Space::Local},
            {"World", Scene::Transform::Space::World}
        }
    );

    p_luaState.new_usertype<Scene::Transform>(
        "Transform",
        "GetPosition", &Scene::Transform::GetPosition,
        "GetRotation", &Scene::Transform::GetRotation,
        "GetEuler", &Scene::Transform::GetEuler,
        "GetScale", &Scene::Transform::GetScale,
        "GetMatrix", &Scene::Transform::GetMatrix,
        "IsDirty", &Scene::Transform::IsDirty,
        "GetUp", &Scene::Transform::GetUp,
        "GetRight", &Scene::Transform::GetRight,
        "GetForward", &Scene::Transform::GetForward,
        "SetPosition", &Scene::Transform::SetPosition,
        "SetRotation", &Scene::Transform::SetRotation,
        "SetEuler", &Scene::Transform::SetEuler,
        "SetScale", &Scene::Transform::SetScale,
        "SetMatrix", &Scene::Transform::SetMatrix,
        "MakeDirty", &Scene::Transform::MakeDirty,
        "RecalculateMatrices", &Scene::Transform::RecalculateMatrices,
        "Move", &Scene::Transform::Move,
        "Rotate", &Scene::Transform::Rotate,
        "RotateEuler", &Scene::Transform::RotateEuler,
        "AddScale", &Scene::Transform::AddScale,
        "RotateAxis", &Scene::Transform::RotateAxis,
        "LookAt", &Scene::Transform::LookAt
    );

    LuaClassDefinition transform;
    transform.name = "Transform";

    transform.functions =
    {
        {
            "GetPosition",
            "Vec3",
            {}
        },
        {
            "GetRotation",
            "Quat",
            {}
        },
        {
            "GetEuler",
            "Vec3",
            {}
        },
        {
            "GetScale",
            "Vec3",
            {}
        },
        {
            "GetMatrix",
            "Mat4",
            {}
        },
        {
            "IsDirty",
            "boolean",
            {}
        },
        {
            "GetUp",
            "Vec3",
            {}
        },
        {
            "GetRight",
            "Vec3",
            {}
        },
        {
            "GetForward",
            "Vec3",
            {}
        },
        {
            "SetPosition",
            "void",
            {
                { "position", "Vec3" }
            }
        },
        {
            "SetRotation",
            "void",
            {
                { "rotation", "Quat" }
            }
        },
        {
            "SetEuler",
            "void",
            {
                { "euler", "Vec3" }
            }
        },
        {
            "SetScale",
            "void",
            {
                { "scale", "Vec3" }
            }
        },
        {
            "SetMatrix",
            "void",
            {
                { "matrix", "Mat4" }
            }
        },
        {
            "MakeDirty",
            "void",
            {}
        },
        {
            "RecalculateMatrices",
            "void",
            {}
        },
        {
            "Move",
            "void",
            {
                { "amount", "Vec3" }
            }
        },
        {
            "Rotate",
            "void",
            {
                { "rotation", "Quat" }
            }
        },
        {
            "RotateEuler",
            "void",
            {
                { "euler", "Vec3" }
            }
        },
        {
            "AddScale",
            "void",
            {
                { "scale", "Vec3" }
            }
        },
        {
            "RotateAxis",
            "void",
            {
                { "axis", "Vec3" },
                { "angle", "number" }
            }
        },
        {
            "LookAt",
            "void",
            {
                { "target", "Vec3" }
            }
        }
    };

    LuaBindings::m_luaClassDefinitions.push_back(transform);
}

void Script::LuaBindings::RegisterGLM(sol::state_view p_luaState)
{
    p_luaState.new_usertype<glm::vec3>(
        "Vec3",
        sol::constructors<glm::vec3(float, float, float)>(),
        "x", &glm::vec3::x,
        "y", &glm::vec3::y,
        "z", &glm::vec3::z
    );

    p_luaState.new_usertype<glm::quat>(
        "Quat",
        sol::constructors<glm::quat(float, float, float, float)>(),
        "w", &glm::quat::w,
        "x", &glm::quat::x,
        "y", &glm::quat::y,
        "z", &glm::quat::z
    );

    p_luaState.new_usertype<glm::mat4>(
        "Mat4",
        sol::constructors<glm::mat4(float)>()
    );

    LuaClassDefinition vec3Def;
    vec3Def.name = "Vec3";

    vec3Def.properties =
    {
        { "x", "number" },
        { "y", "number" },
        { "z", "number" }
    };

    LuaBindings::m_luaClassDefinitions.push_back(vec3Def);

    LuaClassDefinition quatDef;
    quatDef.name = "Quat";

    quatDef.properties =
    {
        { "w", "number" },
        { "x", "number" },
        { "y", "number" },
        { "z", "number" }
    };

    LuaBindings::m_luaClassDefinitions.push_back(quatDef);

    LuaClassDefinition mat4Def;
    mat4Def.name = "Mat4";

    LuaBindings::m_luaClassDefinitions.push_back(mat4Def);
}

void Droplet::Script::LuaBindings::RegisterInput(sol::state_view p_luaState)
{
    
	p_luaState.new_enum<Key>(
		"Key",
		{
			{"A", Key::KeyA},
			{"B", Key::KeyB},
			{"C", Key::KeyC},
			{"D", Key::KeyD},
			{"E", Key::KeyE},
			{"F", Key::KeyF},
			{"G", Key::KeyG},
			{"H", Key::KeyH},
			{"I", Key::KeyI},
			{"J", Key::KeyJ},
			{"K", Key::KeyK},
			{"L", Key::KeyL},
			{"M", Key::KeyM},
			{"N", Key::KeyN},
			{"O", Key::KeyO},
			{"P", Key::KeyP},
			{"Q", Key::KeyQ},
			{"R", Key::KeyR},
			{"S", Key::KeyS},
			{"T", Key::KeyT},
			{"U", Key::KeyU},
			{"V", Key::KeyV},
			{"W", Key::KeyW},
			{"X", Key::KeyX},
			{"Y", Key::KeyY},
			{"Z", Key::KeyZ},

			{"1", Key::Key1},
			{"2", Key::Key2},
			{"3", Key::Key3},
			{"4", Key::Key4},
			{"5", Key::Key5},
			{"6", Key::Key6},
			{"7", Key::Key7},
			{"8", Key::Key8},
			{"9", Key::Key9},
			{"0", Key::Key0},

            {"Enter", Key::KeyEnter},
            {"Escape", Key::KeyEscape},
            {"Backspace", Key::KeyBackspace},
            {"Tab", Key::KeyTab},
		    {"Space", Key::KeySpace},
			{"F1", Key::KeyF1},
			{"F2", Key::KeyF2},
			{"F3", Key::KeyF3},
			{"F4", Key::KeyF4},
			{"F5", Key::KeyF5},
			{"F6", Key::KeyF6},
			{"F7", Key::KeyF7},
			{"F8", Key::KeyF8},
			{"F9", Key::KeyF9},
			{"F10", Key::KeyF10},
			{"F11", Key::KeyF11},
			{"F12", Key::KeyF12}
		}
	);


    p_luaState.new_usertype<GameInput>(
        "Input",
        "KeyPressed", &GameInput::KeyPressed,
        "KeyHeld", &GameInput::KeyHeld,
        "KeyReleased", &GameInput::KeyReleased
        //"MousePressed", &GameInput::MousePressed,
        //"MouseHeld", &GameInput::MouseHeld,
        //"MouseReleased", &GameInput::MouseReleased,
        //"GetCursorX", &GameInput::GetCursorX,
        //"GetCursorY", &GameInput::GetCursorY
    );
}

