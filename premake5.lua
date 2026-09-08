require "Premake/Utilities/Clean"
require "Premake/Utilities/Helper"

workspace "DropletEngine"

    location "Generated"
    cppdialect "C++23"
    warnings "Extra"
    fatalwarnings { "All" }
    configurations { "debug", "release" }

    architecture "x86_64"
    staticruntime "on"

    filter "configurations:debug"
        runtime "Debug"
        defines { "DEBUG" }
        symbols "On"
        optimize "Off"
    filter "configurations:release"
        runtime "Release"
        defines { "NDEBUG" }
        optimize "On"

    -- Paths

    rootPath = path.getdirectory(_SCRIPT)
    targetBuildPath = path.getdirectory(_SCRIPT) .. "/Build/Target"
    objBuildPath = path.getdirectory(_SCRIPT) .. "/Build/Obj"
    includePath = path.getdirectory(_SCRIPT) .. "/include"
    srcPath = path.getdirectory(_SCRIPT) .. "/src"
    externalPath = path.getdirectory(_SCRIPT) .. "/External"
    projectsPath = path.getdirectory(_SCRIPT) .. "/Generated"

include "Premake/Engine"
include "Premake/Test"

filter "system:windows"
    include "Premake/External/Windows/GoogleTest"

filter "system:linux"
    include "Premake/External/Linux/GoogleTest"
