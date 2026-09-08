project "Engine"
    kind "StaticLib"

    location(projectsPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")
    includedirs {"include/", "include/**",  targetBuildPath .. "/External/include/", "."}
    libdirs {targetBuildPath .. "/External/lib/", targetBuildPath .. "/External/lib64/"}
    --dependson{"SDL3", "ImGui"}
    --buildoptions { "-FIEnginePCH.hpp" }

    files {
        AddQuotation(rootPath .. "/include/**.hpp"),
        AddQuotation(rootPath .. "/src/**.cpp"),
    }

    --pchheader "%{prj.location}/EnginePCH.hpp"
    --pchsource "%{prj.location}/EnginePCH.cpp"