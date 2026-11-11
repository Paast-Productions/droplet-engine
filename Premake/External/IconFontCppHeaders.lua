project "IconFontCppHeaders"
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

    filter "system:windows"
        kind "Utility"

        prebuildcommands
        {
            "{MKDIR} " .. AddQuotation(IconFontPath),
            "{COPYFILE} " .. AddQuotation(rootPath .. "/External/IconFontCppHeaders/IconsLucide.h") .. " " .. AddQuotation(IconFontPath)
        }

    filter "system:linux"
        kind "Makefile"

    filter { "system:linux", "configurations:editor-debug or game-debug" }
        buildcommands
        {
            "{MKDIR} " .. AddQuotation(IconFontPath),
            "{COPYFILE} " .. AddQuotation(rootPath .. "/External/IconFontCppHeaders/IconsLucide.h") .. " " .. AddQuotation(IconFontPath)
        }

    filter { "system:linux", "configurations:editor-release or editor-deploy or game-release or game-deploy" }
        buildcommands
        {
            "{MKDIR} " .. AddQuotation(IconFontPath),
            "{COPYFILE} " .. AddQuotation(rootPath .. "/External/IconFontCppHeaders/IconsLucide.h") .. " " .. AddQuotation(IconFontPath)
        }

    filter ""