project "Vulkan-HelloTriangle"

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
        rootPath .. "/Samples/Vulkan/HelloTriangle.cpp",
    }

    if _TARGET_OS == "windows" then
        local vkPath = os.getenv("VULKAN_SDK")
    
        libdirs
        {
            targetBuildPath .. "/Engine",
            vkPath .. "/Lib",
            targetBuildPath .. "/External/lib",
            targetBuildPath .. "/External/lib64"
        }
    
        includedirs
        {
            rootPath .. "/include",
            vkPath .. "/Include",
            targetBuildPath .. "/External/include"
        }
    
        links {
            "Engine",
            "ImGui",
            "VulkanMemoryAllocator",
            "tracy",
            AddQuotation("SDL3"),
            AddQuotation("Shaderc"),
            AddQuotation("Slangd"),
            AddQuotation("lua-5.4.7"),
            AddQuotation("vulkan-1")
        }
    else
        libdirs
        {
            targetBuildPath .. "/Engine"
        }
    
        includedirs
        {
            rootPath .. "/include",
            targetBuildPath .. "/External/include"
        }
    
        links {
            "Engine",
            "ImGui",
            "VulkanMemoryAllocator",
            AddQuotation("SDL3"),
            AddQuotation("vulkan"),
            AddQuotation("lua-5.4.7")
        }
    end
    
    dependson {
        "Engine",
        "ImGui",
        "tracy",
        "Sol2"
    }