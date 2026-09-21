project "AssetManager-AssetLoading"

    kind "ConsoleApp"
    location(projectPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    debugdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")
    
    -- EXPLICITLY ADD WHICH FILES ARE RELEVANT
    files {
        rootPath .. "/Samples/AssetManager/AssetLoading.cpp",
        rootPath .. "/src/asset/AssetCatalog.cpp",
        rootPath .. "/src/asset/AssetManager.cpp",
        rootPath .. "/src/asset/ResourceTexture.cpp"
    }

    libdirs {
        targetBuildPath .. "/Engine",
        targetBuildPath .. "/External/lib",

    }

        local vkPath = os.getenv("VULKAN_SDK")

    includedirs {
        rootPath .. "/include",
        vkPath .. "/Include",
        targetBuildPath .. "/External/include"
    }

    dependson {
        "Engine",
        "Assimp",
        "json",
	"Gli"
    }

    links {
        "Engine",
        AddQuotation("zlibstaticd"),
        AddQuotation("assimp-vc145-mtd")
    }
    defines
    {
        "GLM_ENABLE_EXPERIMENTAL"
    }