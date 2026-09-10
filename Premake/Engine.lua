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
	"Jolt"
    }
    --buildoptions { "-FIEnginePCH.hpp" }

    files {
        "../include/**.hpp",
        "../src/**.cpp"
    }

    --pchheader "%{prj.location}/EnginePCH.hpp"
    --pchsource "%{prj.location}/EnginePCH.cpp"