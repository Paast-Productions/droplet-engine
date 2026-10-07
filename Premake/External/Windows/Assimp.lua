-- TODO: Compile only for the formats we actually need.
project "Assimp"
    kind "StaticLib"
    location(projectsPath)

    local moduleDirectory = AddQuotation(externalPath .. "/%{prj.name}")

    targetdir(targetBuildPath .. "/External")
    objdir(objBuildPath .. "/%{prj.name}")
    
    kind "Utility"

    filter "configurations:release"
        prebuildcommands{
            "{MKDIR} %{prj.objdir}",
            "cmake -S " .. moduleDirectory .. " -B %{prj.objdir} -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=%{prj.targetdir} -DCMAKE_MSVC_RUNTIME_LIBRARY='MultiThreaded' -DASSIMP_BUILD_TESTS=OFF -DLIBRARY_SUFFIX='-mt'",
            "cmake --build %{prj.objdir} --config %{cfg.buildcfg} --target install",
        }
    filter "configurations:debug"
        prebuildcommands{
            "{MKDIR} %{prj.objdir}",
            "cmake -S " .. moduleDirectory .. " -B %{prj.objdir} -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=%{prj.targetdir} -DCMAKE_MSVC_RUNTIME_LIBRARY='MultiThreadedDebug' -DASSIMP_BUILD_TESTS=OFF -DLIBRARY_SUFFIX='-mt'",
            "cmake --build %{prj.objdir} --config %{cfg.buildcfg} --target install",
        }