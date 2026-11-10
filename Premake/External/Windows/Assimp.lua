-- TODO: Compile only for the formats we actually need.
project "Assimp"
    kind "StaticLib"
    location(projectsPath)

    local moduleDirectory = AddQuotation(externalPath .. "/%{prj.name}")
    local objDirectory = AddQuotation(objBuildPath .. "/%{prj.name}")
    local installDirectory = AddQuotation(targetBuildPath .. "/External")

    targetdir(targetBuildPath .. "/External")
    objdir(objBuildPath .. "/%{prj.name}")
    
    kind "Utility"

    filter "configurations:release"
        prebuildcommands{
            "{MKDIR} " .. objDirectory,
            "cmake -S " .. moduleDirectory .. " -B " .. objDirectory .. " -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=" .. installDirectory .. " " ..
                    "-DCMAKE_MSVC_RUNTIME_LIBRARY='MultiThreaded' -DASSIMP_BUILD_TESTS=OFF -DLIBRARY_SUFFIX='-mt' "..
                    "-DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=OFF -DASSIMP_BUILD_FBX_IMPORTER=ON -DASSIMP_BUILD_GLTF_IMPORTER=ON -DASSIMP_BUILD_OBJ_IMPORTER=ON " ..
                    "-DASSIMP_NO_EXPORT=ON",
            "cmake --build " .. objDirectory .. " --config %{cfg.buildcfg} --target install",
        }
    filter "configurations:debug"
        prebuildcommands{
            "{MKDIR} " .. objDirectory,
            "cmake -S " .. moduleDirectory .. " -B " .. objDirectory .. " -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=" .. installDirectory .. " " ..
                    "-DCMAKE_MSVC_RUNTIME_LIBRARY='MultiThreadedDebug' -DASSIMP_BUILD_TESTS=OFF -DLIBRARY_SUFFIX='-mt' " ..
                    "-DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=OFF -DASSIMP_BUILD_FBX_IMPORTER=ON -DASSIMP_BUILD_GLTF_IMPORTER=ON -DASSIMP_BUILD_OBJ_IMPORTER=ON " ..
                    "-DASSIMP_NO_EXPORT=ON",
            "cmake --build " .. objDirectory .. " --config %{cfg.buildcfg} --target install",
        }
    filter {}