project "Test"

    location(projectsPath)

    removefatalwarnings { "All" }

    kind "ConsoleApp"
    targetdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")

    local vkPath = os.getenv("VULKAN_SDK")

    files 
    {
        "../Test/src/**.hpp",
        "../Test/src/**.cpp"
    }

    includedirs
    {
        rootPath .. "/include", 
        vkPath .. "/Include",
        targetBuildPath .. "/External/include"
    }
    
    libdirs
    {
        targetBuildPath .. "/External/lib",
        targetBuildPath .. "/External/lib64",
        vkPath .. "/Lib"
    }

    dependson 
    {
        "GoogleTest",
        "Engine",
        "Assimp",
        "json",
        "bvh",
        "stb",
        "Sol2",
        "tracy"
    }

    links
    {
        "Engine",
        "gtest"
    }