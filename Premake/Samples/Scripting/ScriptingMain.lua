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

    if _TARGET_OS == "windows" then

        local vkPath = os.getenv("VULKAN_SDK")

        includedirs
        {
            rootPath .. "/include",
            vkPath .. "/include",
            targetBuildPath .. "/External/include"
        }

		libdirs {
			targetBuildPath .. "/Library",
			targetBuildPath .. "/External/lib",
			targetBuildPath .. "/External/bin",
			vkPath .. "/Lib"
		}

    else

        includedirs
        {
            rootPath .. "/include",
            targetBuildPath .. "/External/include"
        }

		libdirs {
			targetBuildPath .. "/Library",
			targetBuildPath .. "/External/lib",
			targetBuildPath .. "/External/bin"
		}

    end

    dependson {
        "Engine",
        --"ImGui",
        "Sol2",
        "tracy"
    }

    links {
        "Engine",
        "ImGui",
        "tracy",
        "lua-5.4.7",
        "winmm",
        "gdi32",
        "shell32",
        AddQuotation("SDL3"),
        AddQuotation("Shaderc"),
        AddQuotation("Slangd")
    }
