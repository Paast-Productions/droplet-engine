project "IconFontCppHeaders"
    kind "Utility"
    location(projectsPath)
    
    warnings "Off"
    
    targetdir(targetBuildPath .. "/External/lib/")
    objdir(objBuildPath .. "/%{prj.name}")
    
    local IconFontPath = targetBuildPath .. "/External/include/%{prj.name}"
    
    prebuildcommands
    {
        "{MKDIR} " .. AddQuotation(IconFontPath),
        "{COPYFILE} " .. AddQuotation(rootPath .. "/External/IconFontCppHeaders/IconsLucide.h") .. " " .. AddQuotation(IconFontPath)
    }