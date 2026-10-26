project "bvh"
    kind "Utility"
    location(projectsPath)

    warnings "Off"

    targetdir(targetBuildPath .. "/External")
    objdir(objBuildPath .. "/%{prj.name}")
	    
    local bvhPath = targetBuildPath .. "/External/include/bvh/v2"

    prebuildcommands{
        "{MKDIR} " .. AddQuotation(bvhPath),
        "{COPYDIR} " .. AddQuotation(rootPath .. "/External/bvh/src/bvh/v2") .. " " .. AddQuotation(bvhPath)
    }

    postbuildcommands {
        "{RMDIR} " .. AddQuotation(bvhPath .. "/c_api"),
        "{DELETE} " .. AddQuotation(bvhPath .. "/CMakeLists.txt")
    }