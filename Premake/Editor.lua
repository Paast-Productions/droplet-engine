project "Editor"
    kind "ConsoleApp"

    location(projectsPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    debugdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")

    local vkPath = os.getenv("VULKAN_SDK")

    UseEditorPCH()

    defines{ 
		"TRACY_ENABLE", 
		"TRACY_ON_DEMAND",
		"TRACY_FIBERS",
		"TRACY_MEMORY",
        "GLM_ENABLE_EXPERIMENTAL",
		"IMGUI_DEFINE_MATH_OPERATORS",
        "ROOT_PATH=" .. AddQuotation(rootPath)
	}

    if _TARGET_OS == "windows" then

        buildoptions {"/Zi"}
        
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

        filter { "system:windows" }

        files 
        { 
            "../Editor/resources.rc", 
            "../Editor/engine-icon.ico" 
        }
        
        postbuildcommands {
            "call  " .. AddQuotation(rootPath .. "/Assets/Shader/compile_debug_shaders.bat")
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
        "Jolt"
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
        AddQuotation("SDL3"),
        AddQuotation("vulkan-1"),
        AddQuotation("Shaderc"),
        AddQuotation("Slangd"),
        AddQuotation("zlibstaticd"),
        AddQuotation("assimp-mtd"),
		AddQuotation("lua_static"),
		AddQuotation("Jolt")
    }