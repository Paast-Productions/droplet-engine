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
    
    -- EXPLICITLY ADD WHICH FILES ARE RELEVANT
    files {
        rootPath .. "/Samples/Scene/SceneMain.cpp"
    }

    libdirs {
        targetBuildPath .. "/Library"
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
		"bvh"
    }

    links {
        "Engine",
        "tracy",
        "ImGui"
    }
