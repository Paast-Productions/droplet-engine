project "SceneSystem"

    kind "ConsoleApp"
    location(projectPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    debugdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")

	defines{ 
		"TRACY_ENABLE", 
		"TRACY_ON_DEMAND",
        "GLM_ENABLE_EXPERIMENTAL",
        "ROOT_PATH=" .. AddQuotation("../../../..")
	}
    
    local vkPath = os.getenv("VULKAN_SDK")

    UseGraphicsPCH()

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
	
	buildoptions {"/Zi"}

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
        "Lua",
        "Jolt",
        "Assimp",
	    "Gli",
	    "Stb",
        "bvh",
        "PortableFileDialogs",
        "IconFontCppHeaders",
        "Dalia"
    }

    links {
        "Engine",
        "ImGui",
        "tracy",
        "lua_static",
        "winmm",
        "gdi32",
        "shell32",
        "VulkanMemoryAllocator",
        "dalia_engine",
        "ole32",
        "avrt",
        AddQuotation("vulkan-1"),
        AddQuotation("SDL3"),
        AddQuotation("Shaderc"),
        AddQuotation("Jolt")
    }

    filter "configurations:debug"
        links { AddQuotation("Slangd"), AddQuotation("zlibstaticd"), AddQuotation("assimp-mtd") }
    filter "configurations:release"
        links { AddQuotation("Slang"), AddQuotation("zlibstatic"), AddQuotation("assimp-mt") }
    filter {}
