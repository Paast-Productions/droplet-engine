project "Engine"
    kind "StaticLib"

    location(projectsPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")

    includedirs
    {
        rootPath .. "/include",
        rootPath .. "/include/**",
        targetBuildPath .. "/External/include/"
    }

    libdirs {targetBuildPath .. "/External/lib/", targetBuildPath .. "/External/lib64/"}
    --dependson{"SDL3", "ImGui"}
    --buildoptions { "-FIEnginePCH.hpp" }

    files {
        rootPath .. "/include/**.hpp",
        rootPath .. "/src/**.cpp"
    }

    --pchheader "%{prj.location}/EnginePCH.hpp"
    --pchsource "%{prj.location}/EnginePCH.cpp"