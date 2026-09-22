project "Engine"
    kind "StaticLib"

    location(projectsPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")
<<<<<<< HEAD
    
    local vkPath = os.getenv("VULKAN_SDK")
=======

    local vkPath = os.getenv("VULKAN_SDK")

>>>>>>> dev
    includedirs
    {
        "../include",
        "../include/**",
        vkPath .. "/Include",
        targetBuildPath .. "/External/include/"
    }

    dependson
    {
    --"ImGui"
	--"Jolt"
    }
    --buildoptions { "-FIEnginePCH.hpp" }

    files {
        "../include/**.hpp",
        "../src/**.cpp"
    }

    --pchheader "%{prj.location}/EnginePCH.hpp"
    --pchsource "%{prj.location}/EnginePCH.cpp"