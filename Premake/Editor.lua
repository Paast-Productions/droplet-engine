project "Editor"
    kind "ConsoleApp"

    location(projectsPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")

    local vkPath = os.getenv("VULKAN_SDK")
	
    includedirs 
	{
        "../Editor/include",
        "../Editor/include/**",
        vkPath .. "/Include",
        targetBuildPath .. "/External/include/"
    }

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
	
    libdirs 
	{
        targetBuildPath .. "/External/lib",
        targetBuildPath .. "/External/lib64",
        targetBuildPath .. "/Engine",
        vkPath .. "/Lib"
    }

    links 
	{
        "Engine",
        "ImGui",
        AddQuotation("SDL3")
    }