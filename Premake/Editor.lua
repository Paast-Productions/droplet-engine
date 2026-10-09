project "Editor"
    kind "ConsoleApp"

    location(projectsPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    debugdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")

    local vkPath = os.getenv("VULKAN_SDK")

    defines{ 
		"TRACY_ENABLE", 
		"TRACY_ON_DEMAND",
        "GLM_ENABLE_EXPERIMENTAL",
		"IMGUI_DEFINE_MATH_OPERATORS"
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
        "json",
        "tracy"
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
        "VulkanMemoryAllocator",
        "tracy",
        AddQuotation("SDL3"),
        AddQuotation("vulkan-1"),
        AddQuotation("Shaderc"),
        AddQuotation("Slangd"),
        AddQuotation("zlibstaticd"),
        AddQuotation("assimp-mtd"),
		AddQuotation("lua-5.4.7")
    }