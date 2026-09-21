project "Stb"
    kind "Utility"
    location(projectsPath)

    warnings "Off"

    targetdir(targetBuildPath .. "/External/lib/")
    objdir(objBuildPath .. "/%{prj.name}")
    
    local StbPath = targetBuildPath .. "/External/include/%{prj.name}"

    prebuildcommands
    {
        "{MKDIR} " .. AddQuotation(StbPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/Stb/stb_image.h") .. " " .. AddQuotation(StbPath)
    }