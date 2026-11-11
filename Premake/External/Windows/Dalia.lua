project "Dalia"
    kind "StaticLib"
    location(projectsPath)

    local moduleDirectory = AddQuotation(externalPath .. "/%{prj.name}")
    local objDirectory = AddQuotation(objBuildPath .. "/%{prj.name}")
    local installDirectory = AddQuotation(targetBuildPath .. "/External")

    targetdir(targetBuildPath .. "/External")
    objdir(objBuildPath .. "/%{prj.name}")

    print(objDirectory)
    print(moduleDirectory)
    print(installDirectory)
    
    kind "Utility"

    filter "configurations:release or deploy"
        prebuildcommands{
            "{MKDIR} " .. objDirectory,
            "cmake -S " .. moduleDirectory .. " -B " .. objDirectory .. " -DCMAKE_INSTALL_PREFIX=" .. installDirectory .. " -DCMAKE_MSVC_RUNTIME_LIBRARY='MultiThreaded' -DDALIA_BUILD_STUDIO=OFF -DDALIA_BUILD_SANDBOX=OFF",
            "cmake --build " .. objDirectory .. " --config Release --target install",
        }
    filter "configurations:debug"
        prebuildcommands{
            "{MKDIR} " .. objDirectory,
            "cmake -S " .. moduleDirectory .. " -B " .. objDirectory .. " -DCMAKE_INSTALL_PREFIX=" .. installDirectory .. " -DCMAKE_MSVC_RUNTIME_LIBRARY='MultiThreadedDebug' -DDALIA_BUILD_STUDIO=OFF -DDALIA_BUILD_SANDBOX=OFF",
            "cmake --build " .. objDirectory .. " --config Debug --target install",
        }
    filter {}