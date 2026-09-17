project "Scripting-Main"

    kind "ConsoleApp"
    location(projectPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    debugdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")
    
    -- EXPLICITLY ADD WHICH FILES ARE RELEVANT
    files {
        rootPath .. "/Samples/Scripting/ScriptingMain.cpp"
    }

    libdirs {
        targetBuildPath .. "/Library",
        targetBuildPath .. "/External/lib",
        targetBuildPath .. "/External/bin"
    }
    
    
    local vkPath = os.getenv("VULKAN_SDK")

    includedirs {
        rootPath .. "/include",
        vkPath .. "/include",
        targetBuildPath .. "/External/include"
    }

    dependson {
        "Engine",
        --"ImGui",
        "Sol2",
        "Lua"
    }

    links {
        "Engine",
        --"ImGui",
        "lua-5.4.7",
        "winmm",
        "gdi32",
        "shell32",
    }
