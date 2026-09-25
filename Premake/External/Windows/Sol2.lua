project "Sol2"
    kind "Utility"
    location(projectsPath)

    local moduleDirectory = externalPath .. "/%{prj.name}"

    targetdir(targetBuildPath .. "/External")
    objdir(objBuildPath .. "/%{prj.name}")

    filter "configurations:release"
        prebuildcommands{
            "{MKDIR} %{prj.objdir}",
            "cmake -S " .. AddQuotation(moduleDirectory) .. " -B %{prj.objdir} -DBUILD_LUA_AS_DLL=OFF -DSOL2_LUA_VERSION='5.4.7' -DCMAKE_INSTALL_PREFIX=%{prj.targetdir} -DCMAKE_MSVC_RUNTIME_LIBRARY='MultiThreaded'",
            "cmake --build %{prj.objdir} --config %{cfg.buildcfg} --target install",
        }

        postbuildcommands{
            "{COPY} " .. objBuildPath .. "/Sol2/x64/lib/Release " .. targetBuildPath .. "/External/lib",
        }

    filter "configurations:debug"
        prebuildcommands{
            "{MKDIR} %{prj.objdir}",
            "cmake -S " .. AddQuotation(moduleDirectory) .. " -B %{prj.objdir} -DBUILD_LUA_AS_DLL=OFF -DSOL2_LUA_VERSION='5.4.7' -DCMAKE_INSTALL_PREFIX=%{prj.targetdir} -DCMAKE_MSVC_RUNTIME_LIBRARY='MultiThreadedDebug'",
            "cmake --build %{prj.objdir} --config %{cfg.buildcfg} --target install",
        }

        postbuildcommands{
            --"{MKDIR} " .. AddQuotation(targetBuildPath .. "/External/lib"),
            "{COPY} " .. objBuildPath .. "/Sol2/x64/lib/Debug " .. targetBuildPath .. "/External/lib",
        }