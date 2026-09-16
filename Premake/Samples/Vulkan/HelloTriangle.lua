project "Vulkan-HelloTriangle"

    kind "ConsoleApp"
    location(projectPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    debugdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")
    
    -- EXPLICITLY ADD WHICH FILES ARE RELEVANT
    files {
        rootPath .. "/Samples/Vulkan/HelloTriangle.cpp"
    }

    libdirs {
        targetBuildPath .. "/Library"
    }
    
    
    local vkPath = os.getenv("VULKAN_SDK")

    includedirs {
        rootPath .. "/include",
        vkPath .. "/include",
        targetBuildPath .. "/External/include"
    }

    dependson {
        "Engine",
        "ImGui"
    }

    links {
        "Engine",
        "ImGui"
    }