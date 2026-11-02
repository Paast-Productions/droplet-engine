project "Engine-HelloEngine"

    kind "ConsoleApp"
    location(projectPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    debugdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")
    
    -- EXPLICITLY ADD WHICH FILES ARE RELEVANT
    files {
        rootPath .. "/Samples/Engine/HelloEngine.cpp",
    }
        
    local vkPath = os.getenv("VULKAN_SDK")

    UseGraphicsPCH()

    defines{
        "ROOT_PATH=" .. AddQuotation("../../..")
    }
    
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
        "Sol2",
        "Lua",
        "tracy",
        "PortableFileDialogs",
        "IconFontCppHeaders",
        "Jolt"
    }

    links {
        "Engine",
        "ImGui",
        "VulkanMemoryAllocator",
        "tracy",
        AddQuotation("SDL3"),
        AddQuotation("Shaderc"),
        AddQuotation("Slangd"),
        AddQuotation("lua_static"),
        AddQuotation("vulkan-1"),
        AddQuotation("zlibstaticd"),
        AddQuotation("assimp-mtd"),
        AddQuotation("Jolt")
    }