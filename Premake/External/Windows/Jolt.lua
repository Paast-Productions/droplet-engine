project "Jolt"
    kind "StaticLib"
    location(projectsPath)

    local moduleDirectory = AddQuotation(externalPath .. "/%{prj.name}")
    local objDirectory = AddQuotation(objBuildPath .. "/%{prj.name}")
    local installDirectory = AddQuotation(targetBuildPath .. "/External")

    targetdir(targetBuildPath .. "/External")
    objdir(objBuildPath .. "/%{prj.name}")
    
    kind "Utility"

    local joltOptions = table.concat({
        "-DTARGET_UNIT_TESTS=OFF",
        "-DTARGET_HELLO_WORLD=OFF",
        "-DTARGET_PERFORMANCE_TEST=OFF",
        "-DTARGET_SAMPLES=OFF",
        "-DTARGET_VIEWER=OFF",

        "-DFLOATING_POINT_EXCEPTIONS_ENABLED=OFF",
        "-DPROFILER_IN_DEBUG_AND_RELEASE=OFF",
        "-DPROFILER_IN_DISTRIBUTION=OFF",
        "-DDEBUG_RENDERER_IN_DEBUG_AND_RELEASE=OFF",
        "-DDEBUG_RENDERER_IN_DISTRIBUTION=OFF",
        "-DENABLE_OBJECT_STREAM=OFF",
    }, " ")

    filter "configurations:release"
        prebuildcommands{
            "{MKDIR} " .. objDirectory,
            "cmake -S " .. moduleDirectory .. "/Build" .. " -B " .. objDirectory .. " -DCMAKE_INSTALL_PREFIX=" .. installDirectory .. " -DCMAKE_MSVC_RUNTIME_LIBRARY='MultiThreaded' -DTARGET_UNIT_TESTS=OFF -DTARGET_HELLO_WORLD=OFF -DTARGET_PERFORMANCE_TEST=OFF -DTARGET_SAMPLES=OFF -DTARGET_VIEWER=OFF" .. joltOptions,
            "cmake --build " .. objDirectory .. " --config %{cfg.buildcfg} --target install",
        }
    filter "configurations:debug"
        prebuildcommands{
            "{MKDIR} " .. objDirectory,
            "cmake -S " .. moduleDirectory .. "/Build" .. " -B " .. objDirectory .. " -DCMAKE_INSTALL_PREFIX=" .. installDirectory .. " -DCMAKE_MSVC_RUNTIME_LIBRARY='MultiThreadedDebug' -DTARGET_UNIT_TESTS=OFF -DTARGET_HELLO_WORLD=OFF -DTARGET_PERFORMANCE_TEST=OFF -DTARGET_SAMPLES=OFF -DTARGET_VIEWER=OFF" .. joltOptions,
            "cmake --build " .. objDirectory .. " --config %{cfg.buildcfg} --target install",
        }
    filter {}