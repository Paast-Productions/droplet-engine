project "Lua"
    kind "StaticLib"
    language "C"
    location(projectsPath)

    warnings "Off"

    local moduleDirectory = externalPath .. "/Lua"
    local includePath = targetBuildPath .. "/External/include"

    targetdir(targetBuildPath .. "/External/lib")
    objdir(objBuildPath .. "/%{prj.name}")

    targetname "lua_static"

    files {
        moduleDirectory .. "/*.c"
    }

    removefiles {
        moduleDirectory .. "/lua.c",
        moduleDirectory .. "/luac.c",
        moduleDirectory .. "/onelua.c",
        moduleDirectory .. "/ltests.c"
    }

    includedirs {
        moduleDirectory
    }

    postbuildcommands {
        "{MKDIR} " .. AddQuotation(includePath),
        "{COPYFILE} " .. AddQuotation(moduleDirectory .. "/lua.h") .. " " .. AddQuotation(includePath),
        "{COPYFILE} " .. AddQuotation(moduleDirectory .. "/luaconf.h") .. " " .. AddQuotation(includePath),
        "{COPYFILE} " .. AddQuotation(moduleDirectory .. "/lauxlib.h") .. " " .. AddQuotation(includePath),
        "{COPYFILE} " .. AddQuotation(moduleDirectory .. "/lualib.h") .. " " .. AddQuotation(includePath),
    }