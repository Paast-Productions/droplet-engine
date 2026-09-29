project "Sol2"
    kind "Utility"
    location(projectsPath)

    local moduleDirectory = externalPath .. "/%{prj.name}"

    targetdir(targetBuildPath .. "/External")
    objdir(objBuildPath .. "/%{prj.name}")

    filter "configurations:release"
        prebuildcommands{
            "{MKDIR} " .. AddQuotation("%{prj.objdir}"),
            "cmake -S " .. AddQuotation(moduleDirectory) .. " -B " .. AddQuotation("%{prj.objdir}") .. " -DBUILD_LUA_AS_DLL=OFF -DSOL2_LUA_VERSION=5.4.7 -DCMAKE_INSTALL_PREFIX=" .. AddQuotation("%{prj.targetdir}") .. " -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded",
            "cmake --build " .. AddQuotation("%{prj.objdir}") .. " --config %{cfg.buildcfg} --target install",
        }

        postbuildcommands{
            "{COPY} " .. AddQuotation(objBuildPath .. "/Sol2/x64/lib/Release") .. " " .. AddQuotation(targetBuildPath .. "/External/lib"),
        }

    filter "configurations:debug"
        prebuildcommands{
            "{MKDIR} " .. AddQuotation("%{prj.objdir}"),
            "cmake -S " .. AddQuotation(moduleDirectory) .. " -B " .. AddQuotation("%{prj.objdir}") .. " -DBUILD_LUA_AS_DLL=OFF -DSOL2_LUA_VERSION=5.4.7 -DCMAKE_INSTALL_PREFIX=" .. AddQuotation("%{prj.targetdir}") .. " -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug",
            "cmake --build " .. AddQuotation("%{prj.objdir}") .. " --config %{cfg.buildcfg} --target install",
        }

        postbuildcommands{
            "{COPY} " .. AddQuotation(objBuildPath .. "/Sol2/x64/lib/Debug") .. " " .. AddQuotation(targetBuildPath .. "/External/lib"),
        }