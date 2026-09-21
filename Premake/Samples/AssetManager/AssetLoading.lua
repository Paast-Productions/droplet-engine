project "AssetManager-AssetLoading"

    kind "ConsoleApp"
    location(projectPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    debugdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")
    
    -- EXPLICITLY ADD WHICH FILES ARE RELEVANT
    files {
        rootPath .. "/Samples/AssetManager/AssetLoading.cpp"
    }

    libdirs {
        targetBuildPath .. "/Engine",
        targetBuildPath .. "/External/lib"
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
        "json"
    }

    links {
        "Engine",
        AddQuotation("zlibstaticd"),
        AddQuotation("assimp-vc145-mtd")
    }

    postbuildcommands {
        '{COPY} "' .. targetBuildPath .. '/External/bin/assimp-vc145-mtd.dll" "%{cfg.targetdir}"'
    }