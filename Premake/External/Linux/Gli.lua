project "Gli"
    kind "StaticLib"
    location(projectsPath)

    local moduleDirectory = AddQuotation(externalPath .. "/%{prj.name}")

    targetdir(targetBuildPath .. "/External")
    objdir(objBuildPath .. "/%{prj.name}")
    
    kind "Makefile"

    buildcommands{
        "{MKDIR} %{prj.objdir}",
        "cmake -S " .. moduleDirectory .. " -B %{prj.objdir} -DCMAKE_INSTALL_PREFIX=%{prj.targetdir} ",
        "cmake --build %{prj.objdir} --config %{cfg.buildcfg} --target install",
    }