project "Game"
    kind "ConsoleApp"

    location(projectsPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    debugdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")

    local vkPath = os.getenv("VULKAN_SDK")

    UseGamePCH()

    defines{ 
        "_GAME",
		"TRACY_ENABLE", 
		"TRACY_ON_DEMAND",
		"TRACY_FIBERS",
		"TRACY_MEMORY",
        "GLM_ENABLE_EXPERIMENTAL",
        "ROOT_PATH=" .. AddQuotation("../../../..")
	}

    if _TARGET_OS == "windows" then

        buildoptions {"/Zi"}

        entrypoint "mainCRTStartup"
        
        includedirs
        {
            "../include",
            "../include/**",
            "../Game/include",
            "../Game/include/**",
            vkPath .. "/Include",
            targetBuildPath .. "/External/include/"
        }

        libdirs
        {
            targetBuildPath .. "/External/lib",
            targetBuildPath .. "/External/lib64",
            targetBuildPath .. "/Engine",
            vkPath .. "/Lib"
        }

        files 
        { 
            "../Game/resources.rc", 
            "../Game/drone-icon.ico" 
        }
        
        postbuildcommands {
            "call " .. AddQuotation(rootPath .. "/Assets/Shader/compile_debug_shaders.bat") .. " Game " .. "%{cfg.buildcfg}"
        }

    else

        includedirs
        {
            "../include",
            "../include/**",
            "../Game/include",
            "../Game/include/**",
            targetBuildPath .. "/External/include/"
        }

        libdirs
        {
            targetBuildPath .. "/External/lib",
            targetBuildPath .. "/External/lib64",
            targetBuildPath .. "/Engine"
        }

    end

    dependson 
    {
        "Engine",
        "json",
        "tracy",
        "Lua",
        "Jolt",
        "Dalia"
    }

    files 
    {
        "../Game/include/**.hpp",
        "../Game/src/**.cpp"
    }

    links 
    {
        "Engine",
        "VulkanMemoryAllocator",
        "tracy",
        "PortableFileDialogs",
        "dalia_engine",
        "ole32",
        "avrt",
        AddQuotation("SDL3"),
        AddQuotation("vulkan-1"),
        AddQuotation("Shaderc"),
		AddQuotation("lua_static"),
		AddQuotation("Jolt")
    }

    filter "configurations:debug"
		links { AddQuotation("Slangd"), AddQuotation("zlibstaticd"), AddQuotation("assimp-mtd") }
    filter "configurations:release or deploy"
		links { AddQuotation("Slang"), AddQuotation("zlibstatic"), AddQuotation("assimp-mt") }
    filter {}