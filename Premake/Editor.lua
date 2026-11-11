project "Editor"
    kind "WindowedApp"

    location(projectsPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    debugdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")

    local vkPath = os.getenv("VULKAN_SDK")

    UseEditorPCH()

    defines{ 
        "_EDITOR",
		"TRACY_ENABLE", 
		"TRACY_ON_DEMAND",
		"TRACY_FIBERS",
		"TRACY_MEMORY",
        "GLM_ENABLE_EXPERIMENTAL",
		"IMGUI_DEFINE_MATH_OPERATORS",
        "ROOT_PATH=" .. AddQuotation("../../../..")
	}

    if _TARGET_OS == "windows" then

        buildoptions {"/Zi"}
        
        entrypoint "mainCRTStartup"

        includedirs
        {
            "../include",
            "../include/**",
            "../Editor/include",
            "../Editor/include/**",
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
            "../Editor/resources.rc", 
            "../Editor/engine-icon.ico" 
        }
        
        postbuildcommands {
            "call " .. AddQuotation(rootPath .. "/Assets/Shader/compile_debug_shaders.bat") .. " Editor " .. "%{cfg.buildcfg}"
        }
    else

        includedirs
        {
            "../include",
            "../include/**",
            "../Editor/include",
            "../Editor/include/**",
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
        "ImGui",
        "ImGuizmo",
        "json",
        "tracy",
        "IconFontCppHeaders",
        "PortableFileDialogs",
        "Lua",
        "Jolt",
        "Dalia"
    }

    files 
    {
        "../Editor/include/**.hpp",
        "../Editor/src/**.cpp"
    }

    links 
    {
        "Engine",
        "ImGui",
        "ImGuizmo",
        "VulkanMemoryAllocator",
        "tracy",
        "IconFontCppHeaders",
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

    filter "configurations:editor-debug"
		links { AddQuotation("Slangd"), AddQuotation("zlibstaticd"), AddQuotation("assimp-mtd") }
    filter "configurations:editor-release or editor-deploy"
		links { AddQuotation("Slang"), AddQuotation("zlibstatic"), AddQuotation("assimp-mt") }
    filter {}