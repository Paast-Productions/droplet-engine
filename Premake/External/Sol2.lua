-- Strangely long compile time at the moment. Investigate fruther.
project "Sol2"
    kind "Utility"
    location(projectsPath)

    local moduleDirectory = externalPath .. "/%{prj.name}"
    local includePath = targetBuildPath .. "/External/include/sol"

    objdir(objBuildPath .. "/%{prj.name}")

    prebuildcommands {
        "{MKDIR} " .. AddQuotation(includePath),
        "{COPYDIR} " .. AddQuotation(moduleDirectory .. "/include/sol") .. " " .. AddQuotation(includePath)
    }