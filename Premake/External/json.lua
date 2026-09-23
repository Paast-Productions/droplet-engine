project "json"
    kind "Utility"
    location(projectsPath)

    warnings "Off"

    targetdir(targetBuildPath .. "/External/lib/")
    objdir(objBuildPath .. "/%{prj.name}")
    
    local jsonPath = targetBuildPath .. "/External/include/%{prj.name}"

    prebuildcommands
    {
        "{MKDIR} " .. AddQuotation(jsonPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/json/single_include/nlohmann/json.hpp") .. " " .. AddQuotation(jsonPath)
    }