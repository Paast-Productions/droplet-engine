project "ResourceManager-ResourceLoading"

    kind "ConsoleApp"
    location(projectPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    debugdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")
    
    -- EXPLICITLY ADD WHICH FILES ARE RELEVANT
    files {
        rootPath .. "/Samples/ResourceManager/ResourceLoading.cpp",
        rootPath .. "/src/resource/ResourceManager.cpp",
        rootPath .. "/src/resource/loaders/GliLoader.cpp",
	    rootPath .. "/src/resource/meta/MetaUtils.cpp",
        rootPath .. "/src/core/ThreadPool.cpp",
        rootPath .. "/src/Debug/Logger.cpp"
    }
    local vkPath = os.getenv("VULKAN_SDK")

    UseGraphicsPCH()

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
	    "Stb",
	"PortableFileDialogs",
	"IconFontCppHeaders"
    }

    links {
        "Engine"
    }

    filter "configurations:debug"
        links { AddQuotation("Slangd"), AddQuotation("zlibstaticd"), AddQuotation("assimp-mtd") }
    filter "configurations:release"
        links { AddQuotation("Slang"), AddQuotation("zlibstatic"), AddQuotation("assimp-mt") }
    filter {}

    defines {
        "GLM_ENABLE_EXPERIMENTAL",
        "ROOT_PATH=" .. AddQuotation("../../../..")
    }

    postbuildcommands {
        "{COPYDIR} \"" .. rootPath .. "/Samples/ResourceManager/assets\" \"%{cfg.targetdir}/assets\""
    }