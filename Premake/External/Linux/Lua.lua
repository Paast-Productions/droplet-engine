project "Lua"
    kind "StaticLib"
    location(projectsPath)

    local moduleDirectory = externalPath .. "/%{prj.name}"

    targetdir(targetBuildPath .. "/External")
    objdir(objBuildPath .. "/%{prj.name}")
    
    kind "Makefile"

    buildcommands{
        "{MKDIR} %{prj.objdir}",
        "cmake -S " .. moduleDirectory .. " -B %{prj.objdir} -DLUA_BUILD_AS_CXX=ON -DLUA_ENABLE_SHARED=OFF -DLUA_ENABLE_TESTING=OFF -DCMAKE_INSTALL_PREFIX=%{prj.targetdir} ",
        "cmake --build %{prj.objdir} --config %{cfg.buildcfg} --target install",
    }