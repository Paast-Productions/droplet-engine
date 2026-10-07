project "SceneSystem"

    kind "ConsoleApp"
    location(projectPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    debugdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")

	defines{ 
		"TRACY_ENABLE", 
		"TRACY_ON_DEMAND",
        "GLM_ENABLE_EXPERIMENTAL"
	}
    
    local vkPath = os.getenv("VULKAN_SDK")

    -- EXPLICITLY ADD WHICH FILES ARE RELEVANT
    files {
        rootPath .. "/Samples/Scene/SceneMain.cpp"
    }

    libdirs {
        targetBuildPath .. "/Library",
        targetBuildPath .. "/External/lib",
        targetBuildPath .. "/External/bin",
        vkPath .. "/Lib"
    }
    
    if _TARGET_OS == "windows" then

        local vkPath = os.getenv("VULKAN_SDK")

        includedirs
        {
            rootPath .. "/include",
            vkPath .. "/Include",
            targetBuildPath .. "/External/include"
        }

    else

        includedirs
        {
            rootPath .. "/include",
            targetBuildPath .. "/External/include"
        }

    end


    dependson {
        "Engine",
        "ImGui",
        "tracy",
        "Sol2",
		    "bvh"
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
