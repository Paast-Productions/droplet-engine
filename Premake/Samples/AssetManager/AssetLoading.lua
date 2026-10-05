project "AssetManager-AssetLoading"

    kind "ConsoleApp"
    location(projectPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    debugdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")
    
    -- EXPLICITLY ADD WHICH FILES ARE RELEVANT
    files {
        rootPath .. "/Samples/AssetManager/AssetLoading.cpp",
        rootPath .. "/src/resource/ResourceManager.cpp",
        rootPath .. "/src/resource/loaders/GliLoader.cpp",
	    rootPath .. "/src/resource/meta/MetaUtils.cpp",
        rootPath .. "/src/core/ThreadPool.cpp",
        rootPath .. "/src/Debug/Logger.cpp"
    }
    local vkPath = os.getenv("VULKAN_SDK")

    libdirs {
        targetBuildPath .. "/Engine",
        targetBuildPath .. "/External/lib",
        vkPath .. "/Lib"
    }

    includedirs {
        rootPath .. "/include",
	    rootPath .. "/include/resource/meta",
        vkPath .. "/Include",
        targetBuildPath .. "/External/include"
    }

    dependson {
        "Engine",
        "Assimp",
        "json",
	    "Gli",
	    "bvh",
	    "Stb"
    }

    links {
        "Engine",
        AddQuotation("zlibstaticd"),
        AddQuotation("assimp-vc145-mtd"),
        AddQuotation("Slangd")
    }

    defines {
        "GLM_ENABLE_EXPERIMENTAL"
    }