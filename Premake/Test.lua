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
        "stb"
    }

    links
    {
        "Engine",
        "gtest",
        AddQuotation("zlibstaticd"),
        AddQuotation("assimp-vc145-mtd"),
        AddQuotation("Slangd")
    }

    defines
    {
        "GLM_ENABLE_EXPERIMENTAL",
        'TEST_ASSET_DIR="' .. path.getabsolute("../Test/src/ResourceManager/Assets") .. '"'
    }