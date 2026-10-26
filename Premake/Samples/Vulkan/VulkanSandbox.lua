project "Vulkan-Sandbox"

    kind "ConsoleApp"
    location(projectPath)


    targetdir(targetBuildPath .. "/%{prj.name}")
    debugdir(targetBuildPath .. "/%{prj.name}")
    objdir(objBuildPath .. "/%{prj.name}")



    -- EXPLICITLY ADD WHICH FILES ARE RELEVANT
    files {
        rootPath .. "/Samples/Vulkan/Sandbox/include/*.hpp",
        rootPath .. "/Samples/Vulkan/Sandbox/include/Synchronization/*.hpp",
        rootPath .. "/Samples/Vulkan/Sandbox/*.cpp",
        rootPath .. "/Samples/Vulkan/Sandbox/Synchronization/*.cpp",
        rootPath .. "/Samples/Vulkan/Sandbox/shaders/*.slang"
    }

    -- Global defines
    defines {
        "VULKAN_HPP_NO_STRUCT_CONSTRUCTORS",
        "VULKAN_HPP_NO_UNION_CONSTRUCTORS"
    }

    if _TARGET_OS == "windows" then
        local vkPath = os.getenv("VULKAN_SDK")
    
        libdirs
        {
            targetBuildPath .. "/Engine",
            vkPath .. "/Lib",
            targetBuildPath .. "/External/lib",
            targetBuildPath .. "/External/lib64"
        }
    
        includedirs
        {
            rootPath .. "/include",
            vkPath .. "/Include",
            targetBuildPath .. "/External/include",
            rootPath .. "/Samples/Vulkan/Sandbox/include",

        }
    
        links {
            "Engine",
            "ImGui",
            "VulkanMemoryAllocator",
            "tracy",
            AddQuotation("SDL3"),
            AddQuotation("Shaderc"),
            AddQuotation("Slangd"),
            AddQuotation("lua-5.4.7"),
            AddQuotation("vulkan-1"),
            AddQuotation("zlibstaticd"),
            AddQuotation("assimp-mtd"),
        }
    else
        libdirs
        {
            targetBuildPath .. "/Engine"
        }
    
        includedirs
        {
            rootPath .. "/include",
            targetBuildPath .. "/External/include"
        }
    
        links {
            "Engine",
            "ImGui",
            "VulkanMemoryAllocator",
            AddQuotation("SDL3"),
            AddQuotation("vulkan"),
            AddQuotation("lua-5.4.7"),
        }
    end
    
    dependson {
        "Engine",
        "ImGui",
        "tracy",
        "Sol2"
    }