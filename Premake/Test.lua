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

    if _TARGET_OS == "windows" then
        local vkPath = os.getenv("VULKAN_SDK")

        includedirs
        {
            "../include",
            vkPath .. "/Include",
            targetBuildPath .. "/External/include"
        }

    else

        includedirs
        {
            "../include",
            targetBuildPath .. "/External/include"
        }

    end

    libdirs
    {
        targetBuildPath .. "/External/lib",
        targetBuildPath .. "/External/lib64"
    }

    dependson 
    {
        "GoogleTest",
        "Engine",
        "Sol2"
    }

    links
    {
        "Engine", 
        "gtest",
        AddQuotation("lua-5.4.7")
    }