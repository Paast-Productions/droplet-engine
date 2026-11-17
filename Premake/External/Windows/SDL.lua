project "SDL"
    location(projectsPath)

    local moduleDirectory = AddQuotation(externalPath .. "/%{prj.name}")
    local objDirectory = AddQuotation(objBuildPath .. "/%{prj.name}")
    local installDirectory = AddQuotation(targetBuildPath .. "/External")

    targetdir(targetBuildPath .. "/External")
    objdir(objBuildPath .. "/%{prj.name}")

    kind "Utility"

    local cmakeCommonFlags = " -DCMAKE_INSTALL_PREFIX=" .. installDirectory ..
        " -DSDL_STATIC=ON" ..
        " -DSDL_SHARED=OFF" ..
        " -DSDL_LIBC=ON" ..
        " -DSDL_TEST_LIBRARY=OFF"

    filter "configurations:release or deploy"
    prebuildcommands{
        "{MKDIR} " .. objDirectory,
        "cmake -S " .. moduleDirectory .. " -B " .. objDirectory .. " -DCMAKE_MSVC_RUNTIME_LIBRARY='MultiThreaded'" .. cmakeCommonFlags,
        "cmake --build " .. objDirectory .. " --config Release --target install",
    }
    filter "configurations:debug"
    prebuildcommands{
        "{MKDIR} " .. objDirectory,
        "cmake -S " .. moduleDirectory .. " -B " .. objDirectory .. " -DCMAKE_MSVC_RUNTIME_LIBRARY='MultiThreadedDebug'" .. cmakeCommonFlags,
        "cmake --build " .. objDirectory .. " --config Debug --target install",
    }
    filter {}
