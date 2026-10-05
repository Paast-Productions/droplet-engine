#include "LuaBindings.hpp"
#include "Node.hpp"
#include "Transform.hpp"
#include <glm/glm.hpp>
#include "GameInput.hpp"
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_video.h>
#include <ImGui/imgui.h>

#include <print>
#include <filesystem>
#include <tuple>

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
	RegisterImGui(p_luaState);

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

void Script::LuaBindings::RegisterInput(sol::state_view p_luaState)
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

    p_luaState.new_enum<Mouse>(
        "Mouse",
        {
            {"LMB", Mouse::LMB},
            {"RMB", Mouse::RMB}
        }
    );

    p_luaState.new_usertype<GameInput>(
        "Input",
        "KeyPressed", &GameInput::KeyPressed,
        "KeyHeld", &GameInput::KeyHeld,
        "KeyReleased", &GameInput::KeyReleased,
        "KeyToggle", &GameInput::KeyToggle,
        "MousePressed", &GameInput::MousePressed,
        "MouseHeld", &GameInput::MouseHeld,
        "MouseReleased", &GameInput::MouseReleased,
        "GetCursorX", &GameInput::GetCursorX,
        "GetCursorY", &GameInput::GetCursorY,
        "GetDeltaMouseX", &GameInput::GetDeltaMouseX,
        "GetDeltaMouseY", &GameInput::GetDeltaMouseY
        //"SetCursorPosition",          //Add when we can get the sdl window
        //[](GameInput& input, float x, float y)
        //{
        //    
        //    //SDL_Window *window = 
        //    //input.SetCursorPosition(window, x, y);
        //}
    );

    LuaClassDefinition keyDef;
    keyDef.name = "Key";

    keyDef.properties =
    {
        { "A", "Key" },
        { "B", "Key" },
        { "C", "Key" },
        { "D", "Key" },
        { "E", "Key" },
        { "F", "Key" },
        { "G", "Key" },
        { "H", "Key" },
        { "I", "Key" },
        { "J", "Key" },
        { "K", "Key" },
        { "L", "Key" },
        { "M", "Key" },
        { "N", "Key" },
        { "O", "Key" },
        { "P", "Key" },
        { "Q", "Key" },
        { "R", "Key" },
        { "S", "Key" },
        { "T", "Key" },
        { "U", "Key" },
        { "V", "Key" },
        { "W", "Key" },
        { "X", "Key" },
        { "Y", "Key" },
        { "Z", "Key" },

        { "Num1", "Key" },
        { "Num2", "Key" },
        { "Num3", "Key" },
        { "Num4", "Key" },
        { "Num5", "Key" },
        { "Num6", "Key" },
        { "Num7", "Key" },
        { "Num8", "Key" },
        { "Num9", "Key" },
        { "Num0", "Key" },

        { "Enter", "Key" },
        { "Escape", "Key" },
        { "Backspace", "Key" },
        { "Tab", "Key" },
        { "Space", "Key" },

        { "F1", "Key" },
        { "F2", "Key" },
        { "F3", "Key" },
        { "F4", "Key" },
        { "F5", "Key" },
        { "F6", "Key" },
        { "F7", "Key" },
        { "F8", "Key" },
        { "F9", "Key" },
        { "F10", "Key" },
        { "F11", "Key" },
        { "F12", "Key" }
    };

    LuaBindings::m_luaClassDefinitions.push_back(keyDef);

    LuaClassDefinition mouseDef;
    mouseDef.name = "Mouse";

    mouseDef.properties =
    {
        { "LMB", "Mouse" },
        { "RMB", "Mouse" }
    };

    LuaBindings::m_luaClassDefinitions.push_back(mouseDef);

    LuaClassDefinition inputDef;
    inputDef.name = "Input";

    inputDef.functions =
    {
        { "KeyPressed", "boolean", {{ "key", "Key" }} },
        { "KeyHeld", "boolean", {{ "key", "Key" }} },
        { "KeyReleased", "boolean", {{ "key", "Key" }} },
        { "KeyToggle", "boolean", {{ "key", "Key" }} },

        { "MousePressed", "boolean", {{ "button", "Mouse" }} },
        { "MouseHeld", "boolean", {{ "button", "Mouse" }} },
        { "MouseReleased", "boolean", {{ "button", "Mouse" }} },

        { "GetCursorX", "number", {} },
        { "GetCursorY", "number", {} },
        { "GetDeltaMouseX", "number", {} },
        { "GetDeltaMouseY", "number", {} }
    };

    LuaBindings::m_luaClassDefinitions.push_back(inputDef);
}

void Script::LuaBindings::RegisterImGui([[maybe_unused]] sol::state_view p_luaState)
{
	sol::table imgui = p_luaState.create_table("ImGui");

    imgui.set_function(
        "Text",
		[](const std::string &p_text)
		{
			ImGui::TextUnformatted(p_text.c_str());
	    }
    );

    imgui.set_function(
        "Separator",
        []()
        {
			ImGui::Separator();
        }
    );

    imgui.set_function(
        "Spacing",
        []()
        {
            ImGui::Spacing();
        }
    );

    imgui.set_function(
        "Button",
        [](const std::string &p_label)
        {
            return ImGui::Button(p_label.c_str());
        }
    );

    imgui.set_function(
        "Checkbox",
        [](const std::string &p_label, bool p_value)
        {
            bool value = p_value;

            const bool changed = ImGui::Checkbox(p_label.c_str(), &value);

            return std::make_tuple(changed, value);
        }
    );

    imgui.set_function(
        "DragFloat",
        [](const std::string& p_label, float p_value, float p_speed, float p_min, float p_max)
        {
            float value = p_value;

            const bool changed = ImGui::DragFloat(
                p_label.c_str(),
                &value,
                p_speed,
                p_min,
                p_max
            );
            return std::make_tuple(changed, value);
        }
    );

    imgui.set_function(
        "DragInt",
        [](const std::string& p_label, int p_value, float p_speed, int p_min, int p_max)
        {
            int value = p_value;

            const bool changed = ImGui::DragInt(
                p_label.c_str(),
                &value,
                p_speed,
                p_min,
                p_max
            );
            return std::make_tuple(changed, value);
        }
    );
}

