project "Dalia"
    kind "StaticLib"
    location(projectsPath)

    local moduleDirectory = AddQuotation(externalPath .. "/%{prj.name}")

    targetdir(targetBuildPath .. "/External")
    objdir(objBuildPath .. "/%{prj.name}")
    
    kind "Utility"

    filter "configurations:release"
        prebuildcommands{
            "{MKDIR} %{path.getabsolute(prj.objdir)}",
            "cmake -S " .. moduleDirectory .. " -B %{path.getabsolute(prj.objdir)} -DCMAKE_INSTALL_PREFIX=%{prj.targetdir} -DCMAKE_MSVC_RUNTIME_LIBRARY='MultiThreaded' -DDALIA_BUILD_STUDIO=OFF -DDALIA_BUILD_SANDBOX=OFF",
            "cmake --build %{path.getabsolute(prj.objdir)} --config %{cfg.buildcfg} --target install",
        }
    filter "configurations:debug"
        prebuildcommands{
            "{MKDIR} %{path.getabsolute(prj.objdir)}",
            "cmake -S " .. moduleDirectory .. " -B %{path.getabsolute(prj.objdir)} -DCMAKE_INSTALL_PREFIX=%{prj.targetdir} -DCMAKE_MSVC_RUNTIME_LIBRARY='MultiThreadedDebug' -DDALIA_BUILD_STUDIO=OFF -DDALIA_BUILD_SANDBOX=OFF",
            "cmake --build %{path.getabsolute(prj.objdir)} --config %{cfg.buildcfg} --target install",
        }