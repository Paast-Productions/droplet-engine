require "Premake/clean"
require "Premake/helper"

workspace "HexEngine"

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
    -- 'Temporary' solution to valgrind not being able to read AVX512 instructions
    filter { "system:linux" }
        buildoptions{"-mavx2"}


    rootPath = path.getdirectory(_SCRIPT)
    targetBuildPath = path.getdirectory(_SCRIPT) .. "/Build/target"
    objBuildPath = path.getdirectory(_SCRIPT) .. "/Build/obj"
    projectsPath = path.getdirectory(_SCRIPT) .. "/Generated"

include "External"
include "Test"
