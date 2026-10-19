project "PortableFileDialogs"
    kind "Utility"
    location(projectsPath)

    warnings "Off"
    
    targetdir(targetBuildPath .. "/External/lib/")
    objdir(objBuildPath .. "/%{prj.name}")

    local PFDPath = targetBuildPath .. "/External/include/%{prj.name}"
    
    prebuildcommands
    {
        "{MKDIR} " .. AddQuotation(PFDPath),
        "{COPYFILE} " .. AddQuotation(rootPath .. "/External/portable-file-dialogs/portable-file-dialogs.h") .. " " .. AddQuotation(PFDPath)
    }