project "Engine"
    kind "StaticLib"

    location(projectsPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")

    local vkPath = os.getenv("VULKAN_SDK")

	defines{ "TRACY_ENABLE", "TRACY_ON_DEMAND" }
	
	filter {"system:windows"}
		buildoptions {"/Zi"}
	
	filter {}

    includedirs
    {
        "../include",
        "../include/**",
        vkPath .. "/Include",
        targetBuildPath .. "/External/include/"
    }

    libdirs
    {
        targetBuildPath .. "/External/lib/",
        targetBuildPath .. "/External/lib64/"
    }

    dependson
    {
        "GoogleTest",
        "ImGui",
        "json",
        "Jolt",
        "Assimp",
        "Stb",
        "Gli",
        "bvh",
        "tracy",
        "Sol2",
        "VulkanMemoryAllocator"
    }
    --buildoptions { "-FIEnginePCH.hpp" }

    files {
        "../include/**.hpp",
        "../include/**.inl",
        "../src/**.cpp",
    }

    defines {
        "GLM_ENABLE_EXPERIMENTAL"
    }
    --pchheader "%{prj.location}/EnginePCH.hpp"
    --pchsource "%{prj.location}/EnginePCH.cpp"