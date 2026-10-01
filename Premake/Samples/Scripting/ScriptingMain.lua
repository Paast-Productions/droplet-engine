project "Scripting-Main"

    kind "ConsoleApp"
    location(projectPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    debugdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")

    local vkPath = os.getenv("VULKAN_SDK")
    
    -- EXPLICITLY ADD WHICH FILES ARE RELEVANT
    
    files {
        rootPath .. "/Samples/Scripting/ScriptingMain.cpp"
    }

    libdirs {
        targetBuildPath .. "/Library",
        targetBuildPath .. "/External/lib",
        targetBuildPath .. "/External/bin",
        vkPath .. "/Lib"
    }


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
        AddQuotation("SDL3"),
        AddQuotation("Shaderc"),
        AddQuotation("Slangd")
    }
