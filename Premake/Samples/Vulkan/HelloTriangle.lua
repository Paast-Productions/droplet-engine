project "Vulkan-HelloTriangle"

    kind "ConsoleApp"
    location(projectPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    debugdir(rootPath .. "/Samples/Vulkan/")
    objdir(objBuildPath .. "/%{prj.name}")
    
    -- EXPLICITLY ADD WHICH FILES ARE RELEVANT
    files {
        rootPath .. "/Samples/Vulkan/HelloTriangle.cpp",
    }
        
    local vkPath = os.getenv("VULKAN_SDK")
    
    libdirs {
        targetBuildPath .. "/Engine",
        vkPath .. "/Lib",
        targetBuildPath .. "/External/lib",
        targetBuildPath .. "/External/lib64"
    }


    includedirs {
        rootPath .. "/include",
        vkPath .. "/include",
        targetBuildPath .. "/External/include"
    }

    dependson {
        "Engine",
        "ImGui",
        "tracy",
        "Sol2"
    }

    links {
        "Engine",
        "ImGui",
        "tracy",
        AddQuotation("SDL3"),
        AddQuotation("Shaderc"),
        AddQuotation("Slangd"),
        AddQuotation("lua-5.4.7")
    }