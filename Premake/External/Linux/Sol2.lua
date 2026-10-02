project "Sol2"
    kind "StaticLib"
    location(projectsPath)

    local moduleDirectory = externalPath .. "/%{prj.name}"

    targetdir(targetBuildPath .. "/External")
    objdir(objBuildPath .. "/%{prj.name}")
    
    kind "Makefile"

    buildcommands{
        "{MKDIR} %{prj.objdir}",
        "cmake -S " .. moduleDirectory .. " -B %{prj.objdir} -DBUILD_LUA_AS_DLL=OFF -DSOL2_LUA_VERSION=5.4.7 -DCMAKE_INSTALL_PREFIX=%{prj.targetdir} ",
        "cmake --build %{prj.objdir} --config %{cfg.buildcfg} --target install",
    }

    postbuildcommands{
        "{COPY} " .. AddQuotation(objBuildPath .. "/Sol2/x64/lib/Debug") .. " " .. AddQuotation(targetBuildPath .. "/External/lib"),
    }
