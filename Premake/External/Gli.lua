project "Gli"
    kind "Utility"
    location(projectsPath)

    warnings "Off"

    targetdir(targetBuildPath .. "/External")
    objdir(objBuildPath .. "/%{prj.name}")

    local gliPath = targetBuildPath .. "/External/include/%{prj.name}"
    
    prebuildcommands {
        "{MKDIR} " .. AddQuotation(gliPath),
        "{COPYDIR} " .. AddQuotation(rootPath .. "/External/Gli/gli") .. " " .. AddQuotation(gliPath)
    }