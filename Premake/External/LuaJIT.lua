project "LuaJIT"
    kind "StaticLib"
    language "C"
    location(projectsPath)

    warnings "Off"

    local moduleDirectory = externalPath .. "/%{prj.name}"
    local includePath = targetBuildPath .. "/External/include"

    targetdir(targetBuildPath .. "/External/lib")
    objdir(objBuildPath .. "/%{prj.name}")

    includedirs {
        moduleDirectory
    }

    buildcommands {
        "{CHDIR} " .. moduleDirectory .. "/src", 
        "msvcbuild.bat debug static",
        "{CHDIR} " .. rootPath
    }

    buildoutputs {
        targetBuildPath .. "/External/lib/minilua.lib",
        targetBuildPath .. "/External/lib/lua51.lib",
        targetBuildPath .. "/External/lib/luajit.lib",
        targetBuildPath .. "/External/lib/buildvm.lib"
    }

    postbuildcommands {
        "{MKDIR} " .. AddQuotation(includePath),
        "{COPYFILE} " .. AddQuotation(moduleDirectory .. "/src/lua.h") .. " " .. AddQuotation(includePath),
        "{COPYFILE} " .. AddQuotation(moduleDirectory .. "/src/luaconf.h") .. " " .. AddQuotation(includePath),
        "{COPYFILE} " .. AddQuotation(moduleDirectory .. "/src/lauxlib.h") .. " " .. AddQuotation(includePath),
        "{COPYFILE} " .. AddQuotation(moduleDirectory .. "/src/lualib.h") .. " " .. AddQuotation(includePath),
        "{COPYFILE} " .. AddQuotation(moduleDirectory .. "/src/luajit.h") .. " " .. AddQuotation(includePath),
        "{MOVE} " .. moduleDirectory .. "/src/minilua.lib " .. targetBuildPath .. "/External/lib",
        "{MOVE} " .. moduleDirectory .. "/src/buildvm.lib " .. targetBuildPath .. "/External/lib",
        "{MOVE} " .. moduleDirectory .. "/src/lua51.lib " .. targetBuildPath .. "/External/lib",
        "{MOVE} " .. moduleDirectory .. "/src/luajit.lib " .. targetBuildPath .. "/External/lib",
        
        "{DELETE} " .. moduleDirectory .. "/src/*.pdb",
        "{DELETE} " .. moduleDirectory .. "/src/*.exe",
        "{DELETE} " .. moduleDirectory .. "/src/*.exp",
        "{DELETE} " .. moduleDirectory .. "/src/luajit.h",
        "{DELETE} " .. moduleDirectory .. "/src/luajit_relver.txt",
    }