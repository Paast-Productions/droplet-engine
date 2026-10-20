project "SlugGenerator"
    kind "ConsoleApp"

    location(projectsPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    debugdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")
    local vkPath = os.getenv("VULKAN_SDK")

    includedirs
    {
        rootPath .. "/Devtools/SlugGenerator/include",
        rootPath .. "/Devtools/SlugGenerator/include/**",
        targetBuildPath .. "/External/include/",
        vkPath .. "/Include",
    }

    files
    {
        rootPath .. "/Devtools/SlugGenerator/include/**.hpp",
        rootPath .. "/Devtools/SlugGenerator/src/**.cpp"
    }
    
    dependson
    {
        "Stb"
    }