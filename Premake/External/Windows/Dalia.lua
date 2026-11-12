project "Dalia"
    location(projectsPath)

    local moduleDirectory = AddQuotation(externalPath .. "/%{prj.name}")
    local tracyIncludeDir = AddQuotation(targetBuildPath .. "/External/include/tracy/public")
    local objDirectory = AddQuotation(objBuildPath .. "/%{prj.name}")
    local installDirectory = AddQuotation(targetBuildPath .. "/External")

    targetdir(targetBuildPath .. "/External")
    objdir(objBuildPath .. "/%{prj.name}")

    kind "Utility"
    dependson { "tracy" }

    local cmakeCommonFlags = " -DCMAKE_INSTALL_PREFIX=" .. installDirectory ..
            " -DDALIA_BUILD_STUDIO=OFF" ..
            " -DDALIA_BUILD_SANDBOX=OFF" ..
            " -DDALIA_BUILD_TESTS=OFF" ..
            " -DDALIA_PROFILE_ENABLE=ON" ..
            " -DDALIA_TRACY_INCLUDE_DIR=" .. tracyIncludeDir ..
            " -DTRACY_ENABLE=ON" ..
            " -DTRACY_ON_DEMAND=ON" ..
            " -DTRACY_FIBERS=ON"

    filter "configurations:release"
        prebuildcommands{
            "{MKDIR} " .. objDirectory,
            "cmake -S " .. moduleDirectory .. " -B " .. objDirectory .. " -DCMAKE_MSVC_RUNTIME_LIBRARY='MultiThreaded'" .. cmakeCommonFlags,
            "cmake --build " .. objDirectory .. " --config %{cfg.buildcfg} --target install",
        }
    filter "configurations:debug"
        prebuildcommands{
            "{MKDIR} " .. objDirectory,
            "cmake -S " .. moduleDirectory .. " -B " .. objDirectory .. " -DCMAKE_MSVC_RUNTIME_LIBRARY='MultiThreadedDebug'" .. cmakeCommonFlags,
            "cmake --build " .. objDirectory .. " --config %{cfg.buildcfg} --target install",
        }
    filter {}
