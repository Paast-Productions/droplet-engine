project "Engine"
    kind "StaticLib"

    location(projectsPath)

    defines
    {
        "ROOT_PATH=" .. AddQuotation(rootPath)
    }

    targetdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")

    if _TARGET_OS == "windows" then

        -- vkPath required as windows doesn't add the vulkan sdk to lib path'
        local vkPath = os.getenv("VULKAN_SDK")

        includedirs
        {
            "../include",
            "../include/**",
            vkPath .. "/Include",
            targetBuildPath .. "/External/include/"
        }

    else

        includedirs
        {
            "../include",
            "../include/**",
            targetBuildPath .. "/External/include"
        }

    end

    dependson
    {
        --"GoogleTest",
        "ImGui",
        "Sol2",
        "Lua",
        "json",
        "VulkanMemoryAllocator"
        --"Jolt"
    }
    --buildoptions { "-FIEnginePCH.hpp" }

    files {
        "../include/**.hpp",
        "../src/**.cpp"
    }

    --pchheader "%{prj.location}/EnginePCH.hpp"
    --pchsource "%{prj.location}/EnginePCH.cpp"
