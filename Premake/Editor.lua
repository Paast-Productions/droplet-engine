project "Editor"
    kind "ConsoleApp"

    location(projectsPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")

    local vkPath = os.getenv("VULKAN_SDK")

    if _TARGET_OS == "windows" then

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
        "json"
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
        AddQuotation("SDL3"),
        AddQuotation("vulkan-1"),
        AddQuotation("Shaderc"),
        AddQuotation("Slangd")
    }