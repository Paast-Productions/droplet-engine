project "Transform"

    kind "ConsoleApp"
    location(projectPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    debugdir(rootPath .. "/Samples/Transform/")
    objdir(objBuildPath .. "/%{prj.name}")
    
	defines{ 
		"TRACY_ENABLE", 
		"TRACY_ON_DEMAND",
		"TRACY_FIBERS",
		"TRACY_MEMORY",
        "GLM_ENABLE_EXPERIMENTAL",
		"IMGUI_DEFINE_MATH_OPERATORS",
        "ROOT_PATH=" .. AddQuotation(rootPath)
	}
    
    local vkPath = os.getenv("VULKAN_SDK")


    -- EXPLICITLY ADD WHICH FILES ARE RELEVANT
    files {
        rootPath .. "/Samples/Transform/TransformMain.cpp",
        rootPath .. "/src/Transform.cpp"
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

        includedirs {
            rootPath .. "/include",
            vkPath .. "/include",
            targetBuildPath .. "/External/include"
        }

    else

        includedirs {
            rootPath .. "/include",
            targetBuildPath .. "/External/include"
        }

    end

    dependson {
        "Engine",
        "ImGui",
        "tracy",
        "Sol2",
	"bvh",
	"PortableFileDialogs",
	"IconFontCppHeaders"
    }

    links {
       "Engine",
        "ImGui",
        "tracy",
        "lua-5.4.7",
        "winmm",
        "gdi32",
        "shell32",
        AddQuotation("SDL3"),
        AddQuotation("Shaderc"),
        AddQuotation("Slangd"),
	AddQuotation("zlibstaticd"),
 	AddQuotation("assimp-mtd")

    }
