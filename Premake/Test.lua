project "Test"

    location(projectsPath)

    removefatalwarnings { "All" }

    kind "ConsoleApp"
    targetdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")

	defines{ 
        "GLM_ENABLE_EXPERIMENTAL",
        'TEST_ASSET_DIR="' .. path.getabsolute("../Test/src/resource/assets") .. '"',
        "ROOT_PATH=" .. AddQuotation("../../..")
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
        "Jolt"
    }

    links
    {
        "Engine",
        "gtest",
        "tracy",
        "VulkanMemoryAllocator",
        "ImGui",
        AddQuotation("SDL3"),
        AddQuotation("vulkan-1"),
        AddQuotation("zlibstaticd"),
        AddQuotation("assimp-mtd"),
        AddQuotation("Shaderc"),
        AddQuotation("Slangd"),
		AddQuotation("lua_static"),
        AddQuotation("Jolt")
    }