project "Scripting-Main"

    kind "ConsoleApp"
    location(projectPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    debugdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")

    local vkPath = os.getenv("VULKAN_SDK")
    
    -- EXPLICITLY ADD WHICH FILES ARE RELEVANT
    
    files {
        rootPath .. "/Samples/Scripting/ScriptingMain.cpp"
    }

    libdirs {
        targetBuildPath .. "/Library",
        targetBuildPath .. "/External/lib",
        targetBuildPath .. "/External/bin",
        vkPath .. "/Lib"
    }

    if _TARGET_OS == "windows" then

        local vkPath = os.getenv("VULKAN_SDK")

        includedirs
        {
            rootPath .. "/include",
            vkPath .. "/include",
            targetBuildPath .. "/External/include"
        }

    else

        includedirs
        {
            rootPath .. "/include",
            targetBuildPath .. "/External/include"
        }

    end

    dependson {
        "Engine",
        "ImGui",
        "Sol2",
        "tracy",
	"PortableFileDialogs",
	"IconFontCppHeaders"
    }

    links 
    {
        "Engine",
        "ImGui",
        "ImGuizmo",
        "VulkanMemoryAllocator",
        "tracy",
        "IconFontCppHeaders",
        "PortableFileDialogs",
        AddQuotation("SDL3"),
        AddQuotation("vulkan-1"),
        AddQuotation("Shaderc"),
        AddQuotation("Slangd"),
		AddQuotation("zlibstaticd"),
	 	AddQuotation("assimp-mtd")
    }
