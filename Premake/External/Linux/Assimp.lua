project "Assimp"
    kind "StaticLib"
    location(projectsPath)

    local moduleDirectory = AddQuotation(externalPath .. "/%{prj.name}")

    targetdir(targetBuildPath .. "/External")
    objdir(objBuildPath .. "/%{prj.name}")
    
    kind "Makefile"

    buildcommands
    {
        "{MKDIR} %{prj.objdir}",
        "cmake -S " .. moduleDirectory .. " -B %{prj.objdir} -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX=%{prj.targetdir} " ..
                "-DCMAKE_MSVC_RUNTIME_LIBRARY='MultiThreadedDebug' -DASSIMP_BUILD_TESTS=OFF -DLIBRARY_SUFFIX='-mt' " ..
                "-DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=OFF -DASSIMP_BUILD_FBX_IMPORTER=ON -DASSIMP_BUILD_GLTF_IMPORTER=ON -DASSIMP_BUILD_OBJ_IMPORTER=ON " ..
                "-DASSIMP_NO_EXPORT=ON",
        "cmake --build %{prj.objdir} --config %{cfg.buildcfg} --target install",
    }