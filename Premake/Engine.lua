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

    dependson
    {
        --"GoogleTest",
        "ImGui",
        "Sol2",
        "json",
        "tracy",
        "VulkanMemoryAllocator"
        --"Jolt"
    }
    --buildoptions { "-FIEnginePCH.hpp" }

    files {
        "../include/**.hpp",
        "../src/**.cpp"
    }

    --pchheader "%{prj.location}/EnginePCH.hpp"
    --pchsource "%{prj.location}/EnginePCH.cpp"