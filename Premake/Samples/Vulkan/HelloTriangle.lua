project "Vulkan-HelloTriangle"

    kind "ConsoleApp"
    location(projectPath)


    targetdir(targetBuildPath .. "/%{prj.name}")
    debugdir(rootPath .. "/Samples/Vulkan/")
    objdir(objBuildPath .. "/%{prj.name}")



    -- EXPLICITLY ADD WHICH FILES ARE RELEVANT
    files {
        rootPath .. "/Samples/Vulkan/HelloTriangle.cpp",
    }

    if _TARGET_OS == "windows" then
        local vkPath = os.getenv("VULKAN_SDK")

        libdirs
        {
            targetBuildPath .. "/Engine",
            vkPath .. "/Lib"
        }

        includedirs
        {
            rootPath .. "/include",
            vkPath .. "/Include",
            targetBuildPath .. "/External/include"
        }

        links {
            "Engine",
            "ImGui",
            "VulkanMemoryAllocator",
            AddQuotation("SDL3"),
            AddQuotation("Shaderc"),
            AddQuotation("Slangd"),
            AddQuotation("vulkan-1")
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
            AddQuotation("vulkan")
        }

    end

    dependson {
        "Engine",
        "ImGui",
        "VulkanMemoryAllocator"
    }