project "Test"

    location(projectsPath)

    kind "ConsoleApp"
    targetdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")

    local vkPath = os.getenv("VULKAN_SDK")

    files 
    {
        "../Test/src/**.hpp",
        "../Test/src/**.cpp"
    }

    local vkPath = os.getenv("VULKAN_SDK")

    includedirs
    {
        "../include", 
        vkPath .. "/Include",
        targetBuildPath .. "/External/include"
    }
    
    libdirs
    {
        targetBuildPath .. "/External/lib",
        targetBuildPath .. "/External/lib64"
    }

    dependson 
    {
        "GoogleTest",
        "Engine"
    }

    links
    {
        "Engine",
        "gtest"
    }