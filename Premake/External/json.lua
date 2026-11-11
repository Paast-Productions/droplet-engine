project "json"
    location(projectsPath)

    warnings "Off"

    targetdir(targetBuildPath .. "/External/lib/")
    objdir(objBuildPath .. "/%{prj.name}")
    
    local jsonPath = targetBuildPath .. "/External/include/%{prj.name}"

    filter "system:windows"
        kind "Utility"

        prebuildcommands
        {
            "{MKDIR} " .. AddQuotation(jsonPath),
            "{COPYFILE} " .. AddQuotation(rootPath .. "/External/json/single_include/nlohmann/json.hpp") .. " " .. AddQuotation(jsonPath)
        }

    filter "system:linux"
        kind "Makefile"

    filter { "system:linux", "configurations:debug" }
        buildcommands
        {
            "{MKDIR} " .. AddQuotation(jsonPath),
            "{COPYFILE} " .. AddQuotation(rootPath .. "/External/json/single_include/nlohmann/json.hpp") .. " " .. AddQuotation(jsonPath)
        }

    filter { "system:linux", "configurations:release or deploy" }
        buildcommands
        {
            "{MKDIR} " .. AddQuotation(jsonPath),
            "{COPYFILE} " .. AddQuotation(rootPath .. "/External/json/single_include/nlohmann/json.hpp") .. " " .. AddQuotation(jsonPath)
        }

    filter ""
