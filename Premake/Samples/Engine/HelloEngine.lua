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
        "ROOT_PATH=" .. AddQuotation("../../../..")
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
        "Jolt",
        "Dalia"
    }

    links {
        "Engine",
        "ImGui",
        "VulkanMemoryAllocator",
        "tracy",
        "dalia_engine",
        "ole32",
        "avrt",
        AddQuotation("SDL3"),
        AddQuotation("Shaderc"),
        AddQuotation("lua_static"),
        AddQuotation("vulkan-1"),
        AddQuotation("Jolt")
    }

    filter "configurations:debug"
        links { AddQuotation("Slangd"), AddQuotation("zlibstaticd"), AddQuotation("assimp-mtd") }
    filter "configurations:release"
        links { AddQuotation("Slang"), AddQuotation("zlibstatic"), AddQuotation("assimp-mt") }
    filter {}