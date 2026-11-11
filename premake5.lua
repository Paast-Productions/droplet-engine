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
    configurations { "editor-debug", "editor-release", "editor-deploy", "game-debug", "game-release", "game-deploy" }

    architecture "x86_64"
    staticruntime "on"

    filter "configurations:editor-debug"
        runtime "Debug"
        defines { "_EDITOR", "DEBUG" }
        symbols "On"
        optimize "Off"
    filter "configurations:editor-release"
        runtime "Release"
        defines { "_EDITOR", "NDEBUG" }
        optimize "On"
    filter "configurations:editor-deploy"
        runtime "Release"
        defines { "_EDITOR", "NDEBUG", "DEPLOY" }
		optimize "Full"
		symbols "Off"
    filter "configurations:game-debug"
        runtime "Debug"
        defines { "_GAME", "DEBUG" }
        symbols "On"
        optimize "Off"
    filter "configurations:game-release"
        runtime "Release"
        defines { "_GAME", "NDEBUG" }
        optimize "On"
    filter "configurations:game-deploy"
        runtime "Release"
        defines { "_GAME", "NDEBUG", "DEPLOY" }
        optimize "Full"
        symbols "Off"
    filter {}

-- [[ PATHS ]] --

rootPath = path.getdirectory(_SCRIPT)
targetBuildPath = path.getdirectory(_SCRIPT) .. "/Build/Target/%{cfg.buildcfg}"
objBuildPath = path.getdirectory(_SCRIPT) .. "/Build/Obj/%{cfg.buildcfg}"
includePath = path.getdirectory(_SCRIPT) .. "/include"
srcPath = path.getdirectory(_SCRIPT) .. "/src"
externalPath = path.getdirectory(_SCRIPT) .. "/External"
projectsPath = path.getdirectory(_SCRIPT) .. "/Generated"

-- [[ SOURCE ]] --

filter "configurations:editor-debug or editor-release or editor-deploy"
    include "Premake/Editor"
filter "configurations:game-debug or game-release or game-deploy"
    include "Premake/Game"
filter {}

include "Premake/Test"
include "Premake/Engine"

-- [[ SAMPLES ]] --

group "Samples"

include "Premake/Samples/Engine/HelloEngine"
include "Premake/Samples/Scene/Scene"

group ""

group "Devtools"

include "Premake/Devtools/SlugGenerator"

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
include "Premake/External/Lua"

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
