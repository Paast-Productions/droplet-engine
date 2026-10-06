project "Test"

    location(projectsPath)

    kind "ConsoleApp"
    targetdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")

    files
    {
        "../Test/src/**.hpp",
        "../Test/src/**.cpp"
    }


    local vkPath = os.getenv("VULKAN_SDK")
    
    if _TARGET_OS == "windows" then


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
        targetBuildPath .. "/External/lib64",
        vkPath .. "/Lib"
    }

    dependson 
    {
        "GoogleTest",
        "Engine",
        "Sol2",
        "tracy"
    }

    links
    {
        "Engine", 
        "gtest",
        "tracy",
        "ImGui",
        AddQuotation("lua-5.4.7"),
        AddQuotation("SDL3"),
        AddQuotation("Shaderc"),
        AddQuotation("Slangd")
    }