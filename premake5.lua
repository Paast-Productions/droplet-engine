require "Premake/Utilities/Clean"
require "Premake/Utilities/Helper"
require "Premake/Utilities/Doxygen"

workspace "DropletEngine"
    
    location "Generated"
    cppdialect "C++23"
    multiprocessorcompile "On"
	startproject "Engine"
    warnings "Extra"
    externalanglebrackets "On"
    externalwarnings "Off"
    --fatalwarnings { "All" }
    multiprocessorcompile "On"
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
    filter {}

    -- Paths

    rootPath = path.getdirectory(_SCRIPT)
    targetBuildPath = path.getdirectory(_SCRIPT) .. "/Build/Target"
    objBuildPath = path.getdirectory(_SCRIPT) .. "/Build/Obj"
    includePath = path.getdirectory(_SCRIPT) .. "/include"
    srcPath = path.getdirectory(_SCRIPT) .. "/src"
    externalPath = path.getdirectory(_SCRIPT) .. "/External"
    projectsPath = path.getdirectory(_SCRIPT) .. "/Generated"

-- [[ SOURCE ]] --

include "Premake/Engine"
include "Premake/Editor"
include "Premake/Test"

-- [[ SAMPLES ]] --

group "Samples"

include "Premake/Samples/Engine/HelloEngine"
include "Premake/Samples/Scene/Scene"

group ""

-- [[ EXTERNAL ]] --

group "External"

include "Premake/External/ImGui"
include "Premake/External/Droplet-ImGuizmo"
include "Premake/External/json"
include "Premake/External/tracy"
include "Premake/External/Stb"
include "Premake/External/IconFontCppHeaders"
include "Premake/External/PortableFileDialogs"
include "Premake/External/VulkanMemoryAllocator"
include "Premake/External/bvh"
include "Premake/External/Gli"
include "Premake/External/Sol2"
include "Premake/External/LuaJIT"
--include "Premake/External/Lua"

if _TARGET_OS == 'windows' then
    include "Premake/External/Windows/GoogleTest"
    include "Premake/External/Windows/Assimp"
    include "Premake/External/Windows/Dalia"
    include "Premake/External/Windows/Jolt"
end

if _TARGET_OS == 'linux' then
    include "Premake/External/Linux/GoogleTest"
    include "Premake/External/Linux/Assimp"
    include "Premake/External/Linux/Jolt"
end

group ""
