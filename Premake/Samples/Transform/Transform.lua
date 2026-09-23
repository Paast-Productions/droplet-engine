project "Transform"

    kind "ConsoleApp"
    location(projectPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    debugdir(rootPath .. "/Samples/Vulkan/")
    objdir(objBuildPath .. "/%{prj.name}")
    
    -- EXPLICITLY ADD WHICH FILES ARE RELEVANT
    files {
        rootPath .. "/Samples/Transform/TransformMain.cpp",
        rootPath .. "/src/Transform.cpp"
    }
        
    local vkPath = os.getenv("VULKAN_SDK")
    
    libdirs {
        targetBuildPath .. "/Engine",
        vkPath .. "/Lib"
    }


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
        "ImGui",
        AddQuotation("SDL3"),
        AddQuotation("Shaderc"),
        AddQuotation("Slangd")
    }