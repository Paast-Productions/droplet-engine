project "Jolt"
    kind "StaticLib"
    location(projectsPath)

    local moduleDirectory = externalPath .. "/%{prj.name}"

    targetdir(targetBuildPath .. "/External")
    objdir(objBuildPath .. "/%{prj.name}")
    
    kind "Makefile"

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

    buildcommands{
        "{MKDIR} %{prj.objdir}",
        "cmake -S " .. moduleDirectory .. "/Build" .. " -B %{prj.objdir} -DCMAKE_INSTALL_PREFIX=%{prj.targetdir} " .. joltOptions,
        "cmake --build %{prj.objdir} --config %{cfg.buildcfg} --target install",
    }