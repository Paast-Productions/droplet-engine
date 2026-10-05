project "Editor"
    kind "ConsoleApp"

    location(projectsPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")

    local vkPath = os.getenv("VULKAN_SDK")

	defines{ "TRACY_ENABLE", "TRACY_ON_DEMAND" }
	
	filter {"system:windows"}
		buildoptions {"/Zi"}
	
	filter {}
	
    includedirs 
	{
        "../include",
        "../include/**",
        "../Editor/include",
        "../Editor/include/**",
        vkPath .. "/Include",
        targetBuildPath .. "/External/include/"
    }

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
        "tracy",
        AddQuotation("SDL3")
    }