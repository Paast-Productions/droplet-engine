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

    local libPath = targetBuildPath .. "/External/lib"

    postbuildcommands {
        "{MKDIR} " .. AddQuotation(includePath),
        "{COPYFILE} " .. AddQuotation(moduleDirectory .. "/src/lua.h") .. " " .. AddQuotation(includePath),
        "{COPYFILE} " .. AddQuotation(moduleDirectory .. "/src/luaconf.h") .. " " .. AddQuotation(includePath),
        "{COPYFILE} " .. AddQuotation(moduleDirectory .. "/src/lauxlib.h") .. " " .. AddQuotation(includePath),
        "{COPYFILE} " .. AddQuotation(moduleDirectory .. "/src/lualib.h") .. " " .. AddQuotation(includePath),
        "{COPYFILE} " .. AddQuotation(moduleDirectory .. "/src/luajit.h") .. " " .. AddQuotation(includePath),
        
        "{MOVE} " .. AddQuotation(moduleDirectory .. "/src/minilua.lib") .. " " .. AddQuotation(libPath),
        "{MOVE} " .. AddQuotation(moduleDirectory .. "/src/buildvm.lib") .. " " .. AddQuotation(libPath),
        "{MOVE} " .. AddQuotation(moduleDirectory .. "/src/lua51.lib") .. " " .. AddQuotation(libPath),
        "{MOVE} " .. AddQuotation(moduleDirectory .. "/src/luajit.lib") .. " " .. AddQuotation(libPath),
        
        "{DELETE} " .. AddQuotation(moduleDirectory .. "/src/*.pdb"),
        "{DELETE} " .. AddQuotation(moduleDirectory .. "/src/*.exe"),
        "{DELETE} " .. AddQuotation(moduleDirectory .. "/src/*.exp"),
        "{DELETE} " .. AddQuotation(moduleDirectory .. "/src/luajit.h"),
        "{DELETE} " .. AddQuotation(moduleDirectory .. "/src/luajit_relver.txt"),
    }