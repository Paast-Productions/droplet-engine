project "AssetManager-AssetLoading"

    kind "ConsoleApp"
    location(projectPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    debugdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")
    
    -- EXPLICITLY ADD WHICH FILES ARE RELEVANT
    files {
        rootPath .. "/Samples/AssetManager/AssetLoading.cpp",
        rootPath .. "/src/asset/ResourceCatalog.cpp",
        rootPath .. "/src/asset/ResourceManager.cpp",
        rootPath .. "/src/asset/TextureLoader.cpp",
	      rootPath .. "/src/asset/meta/MetaUtils.cpp"
    }

    libdirs {
        targetBuildPath .. "/Engine",
        targetBuildPath .. "/External/lib",

    }
    
    local vkPath = os.getenv("VULKAN_SDK")

    includedirs {
        rootPath .. "/include",
  	    rootPath .. "/include/asset/meta",
        vkPath .. "/Include",
        targetBuildPath .. "/External/include"
    }

    dependson {
        "Engine",
        "Assimp",
        "json",
	      "Gli",
	      "Stb"
    }

    links {
        "Engine",
        AddQuotation("zlibstaticd"),
        AddQuotation("assimp-vc145-mtd")
    }

    defines {
        "GLM_ENABLE_EXPERIMENTAL"
    }

    postbuildcommands {
        '{COPY} "' .. targetBuildPath .. '/External/bin/assimp-vc145-mtd.dll" "%{cfg.targetdir}"'
    }