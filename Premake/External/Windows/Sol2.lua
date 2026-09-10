project "Sol2"
    kind "StaticLib"
    location(projectsPath)

    local moduleDirectory = externalPath .. "/%{prj.name}"

    targetdir(targetBuildPath .. "/External")
    objdir(objBuildPath .. "/%{prj.name}")
    
    kind "Utility"

    filter "configurations:release"
        prebuildcommands{
            "{MKDIR} %{prj.objdir}",
            "cmake -S " .. AddQuotation(moduleDirectory) .. " -B %{prj.objdir} -DSOL_LUAJIT=ON -DSOL2_BUILD_LUA=ON -DBUILD_LUA_AS_DLL=OFF -DCMAKE_INSTALL_PREFIX=%{prj.targetdir} -DCMAKE_MSVC_RUNTIME_LIBRARY='MultiThreaded'",
            "cmake --build %{prj.objdir} --config %{cfg.buildcfg} --target install",
        }
    filter "configurations:debug"
        prebuildcommands{
            "{MKDIR} %{prj.objdir}",
            "cmake -S " .. AddQuotation(moduleDirectory) .. " -B %{prj.objdir} -DSOL_LUAJIT=ON -DSOL2_BUILD_LUA=ON -DBUILD_LUA_AS_DLL=OFF -DCMAKE_INSTALL_PREFIX=%{prj.targetdir} -DCMAKE_MSVC_RUNTIME_LIBRARY='MultiThreadedDebug'",
            "cmake --build %{prj.objdir} --config %{cfg.buildcfg} --target install",
        }