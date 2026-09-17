project "Engine"
    kind "StaticLib"

    location(projectsPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")

    includedirs
    {
        "../include",
        "../include/**",
        targetBuildPath .. "/External/include/"
    }

    libdirs
    {
        targetBuildPath .. "/External/lib/",
        targetBuildPath .. "/External/lib64/"
    }

    dependson
    {
        "GoogleTest",
        "ImGui",
        "Jolt",
        "json",
        "Assimp"
    }
    --buildoptions { "-FIEnginePCH.hpp" }

    files {
        "../include/**.hpp",
        "../include/**.inl",
        "../src/**.cpp",
        "../src/**.h"
    }

    --pchheader "%{prj.location}/EnginePCH.hpp"
    --pchsource "%{prj.location}/EnginePCH.cpp"