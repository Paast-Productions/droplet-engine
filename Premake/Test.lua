project "Test"

    location(projectsPath)

    removefatalwarnings { "All" }

    kind "ConsoleApp"
    targetdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")

	defines{ 
        "GLM_ENABLE_EXPERIMENTAL",
        'TEST_ASSET_DIR="' .. path.getabsolute("../Test/src/resource/assets") .. '"',
        "ROOT_PATH=" .. AddQuotation("../../../..")
	}

    files
    {
        "../Test/src/**.hpp",
        "../Test/src/**.cpp"
    }

    if _TARGET_OS == "windows" then
        local vkPath = os.getenv("VULKAN_SDK")

        includedirs
        {
            "../include",
            vkPath .. "/Include",
            targetBuildPath .. "/External/include"
        }
    
		libdirs
		{
			targetBuildPath .. "/External/lib",
			targetBuildPath .. "/External/lib64",
			vkPath .. "/Lib"
		}

    else

        includedirs
        {
            "../include",
            targetBuildPath .. "/External/include"
        }
    
		libdirs
		{
			targetBuildPath .. "/External/lib",
			targetBuildPath .. "/External/lib64",
			"Lib"
		}

    end

    dependson 
    {
        "GoogleTest",
        "Engine",
        "Assimp",
        "json",
        "bvh",
        "stb",
        "Sol2",
        "Lua",
        "tracy",
        "Jolt",
        "Dalia"
    }

    links
    {
        "Engine",
        "gtest",
        "tracy",
        "VulkanMemoryAllocator",
        "ImGui",
        "dalia_engine",
        "ole32",
        "avrt",
        AddQuotation("SDL3"),
        AddQuotation("vulkan-1"),
        AddQuotation("Shaderc"),
		AddQuotation("lua_static"),
        AddQuotation("Jolt")
    }

    filter "configurations:debug"
        links { AddQuotation("Slangd"), AddQuotation("zlibstaticd"), AddQuotation("assimp-mtd") }
    filter "configurations:release"
        links { AddQuotation("Slang"), AddQuotation("zlibstatic"), AddQuotation("assimp-mt") }
    filter {}