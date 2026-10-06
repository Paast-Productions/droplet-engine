-- Strangely long compile time at the moment. Investigate fruther.
project "Sol2"
    kind "Utility"
    location(projectsPath)

    local moduleDirectory = externalPath .. "/%{prj.name}"

    targetdir(targetBuildPath .. "/External")
    objdir(objBuildPath .. "/%{prj.name}")

    filter "configurations:release"
        prebuildcommands{
            "{MKDIR} " .. AddQuotation("%{prj.objdir}"),
            "cmake -S " .. AddQuotation(moduleDirectory) .. " -B " .. AddQuotation("%{prj.objdir}") .. " -DSOL2_BUILD_LUA=ON -DBUILD_LUA_AS_DLL=OFF -DSOL2_LUA_VERSION=5.4.7 -DCMAKE_INSTALL_PREFIX=" .. AddQuotation("%{prj.targetdir}") .. " -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded",
            "cmake --build " .. AddQuotation("%{prj.objdir}") .. " --config %{cfg.buildcfg} --target install",
        }

        postbuildcommands {
            "{COPYDIR} " .. AddQuotation(objBuildPath .. "/Sol2/x64/lib/Release") .. " " .. AddQuotation(targetBuildPath .. "/External/lib"),
        }

    filter "configurations:debug"
        prebuildcommands{
            "{MKDIR} " .. AddQuotation("%{prj.objdir}"),
            "cmake -S " .. AddQuotation(moduleDirectory) .. " -B " .. AddQuotation("%{prj.objdir}") .. " -DSOL2_BUILD_LUA=ON -DBUILD_LUA_AS_DLL=OFF -DSOL2_LUA_VERSION=5.4.7 -DCMAKE_INSTALL_PREFIX=" .. AddQuotation("%{prj.targetdir}") .. " -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug",
            "cmake --build " .. AddQuotation("%{prj.objdir}") .. " --config %{cfg.buildcfg} --target install",
        }

        local luaSourcePath = objBuildPath .. "/Sol2/_deps/lua-vanilla-src/src"
        local includePath = targetBuildPath .. "/External/include" -- TODO: Move the headers into a directory?

        postbuildcommands {
            "{COPYFILE} " .. AddQuotation(luaSourcePath .. "/lua.h") .. " " .. AddQuotation(includePath),
            "{COPYFILE} " .. AddQuotation(luaSourcePath .. "/lua.hpp") .. " " .. AddQuotation(includePath),
            "{COPYFILE} " .. AddQuotation(luaSourcePath .. "/luaconf.h") .. " " .. AddQuotation(includePath),
            "{COPYFILE} " .. AddQuotation(luaSourcePath .. "/lauxlib.h") .. " " .. AddQuotation(includePath),
            "{COPYFILE} " .. AddQuotation(luaSourcePath .. "/lualib.h") .. " " .. AddQuotation(includePath),
        
            "{COPYDIR} " .. AddQuotation(objBuildPath .. "/Sol2/x64/lib/Debug") .. " " .. AddQuotation(targetBuildPath .. "/External/lib"),
        }