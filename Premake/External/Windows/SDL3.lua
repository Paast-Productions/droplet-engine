project "SDL3"
    kind "StaticLib"
    location(projectsPath)

    moduleDirectory = AddQuotation(externalPath .. "/%{prj.name}")

    targetdir(targetBuildPath .. "/External")
    objdir(objBuildPath .. "/%{prj.name}")

    kind "Utility"

    filter "configurations:release"
        prebuildcommands
        {
            "{MKDIR} %{prj.objdir}",
            "cmake -S " .. moduleDirectory .. " -B %{prj.objdir} -DCMAKE_INSTALL_PREFIX=%{prj.targetdir} -DSDL_STATIC=ON -DSDL_SHARED=OFF -DSDL_LIBC=ON -DCMAKE_MSVC_RUNTIME_LIBRARY='MultiThreaded'",
            "cmake --build %{prj.objdir} --config %{cfg.buildcfg} --target install",
        }

    filter "configurations:debug"
        prebuildcommands
        {
            "{MKDIR} %{prj.objdir}",
            "cmake -S " .. moduleDirectory .. " -B %{prj.objdir} -DCMAKE_INSTALL_PREFIX=%{prj.targetdir} -DSDL_STATIC=ON -DSDL_SHARED=OFF -DSDL_LIBC=ON -DCMAKE_MSVC_RUNTIME_LIBRARY='MultiThreadedDebug'",
            "cmake --build %{prj.objdir} --config %{cfg.buildcfg} --target install",
        }