project "GLM"
    location(projectsPath)

    local moduleDirectory = AddQuotation(externalPath .. "/%{prj.name}")
    local objDirectory = AddQuotation(objBuildPath .. "/%{prj.name}")
    local installDirectory = AddQuotation(targetBuildPath .. "/External")

    targetdir(targetBuildPath .. "/External")
    objdir(objBuildPath .. "/%{prj.name}")

    kind "Utility"

    local cmakeCommonFlags = " -DCMAKE_INSTALL_PREFIX=" .. installDirectory ..
        " -DGLM_BUILD_INSTALL=ON" ..
        " -DGLM_ENABLE_CXX_20=ON" ..
        " -DGLM_ENABLE_FAST_MATH=ON" ..
        " -DGLM_ENABLE_SIMD_AVX2=ON"

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
