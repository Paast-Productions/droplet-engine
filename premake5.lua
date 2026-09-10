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

--[[ SOURCE ]]--

include "Premake/Engine"
include "Premake/Test"

--[[ EXTERNAL ]]--

include "Premake/External/ImGui"

if _TARGET_OS == 'windows' then
    include "Premake/External/Windows/GoogleTest"
    include "Premake/External/Windows/Assimp"
    include "Premake/External/Windows/Gli"
end

if _TARGET_OS == 'linux' then
    include "Premake/External/Linux/GoogleTest"
    include "Premake/External/Linux/Assimp"
    include "Premake/External/Linux/Gli"
end
