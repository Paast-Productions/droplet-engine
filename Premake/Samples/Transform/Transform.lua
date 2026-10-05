project "Transform"

    kind "ConsoleApp"
    location(projectPath)

    targetdir(targetBuildPath .. "/%{prj.name}")
    debugdir(rootPath .. "/Samples/Transform/")
    objdir(objBuildPath .. "/%{prj.name}")
    
    -- EXPLICITLY ADD WHICH FILES ARE RELEVANT
    files {
        rootPath .. "/Samples/Transform/TransformMain.cpp",
        rootPath .. "/src/Transform.cpp"
    }

    if _TARGET_OS == "windows" then

        local vkPath = os.getenv("VULKAN_SDK")
    
        libdirs {
            targetBuildPath .. "/Engine",
            vkPath .. "/Lib"
        }


        includedirs {
            rootPath .. "/include",
            vkPath .. "/include",
            targetBuildPath .. "/External/include"
        }

    else

        libdirs {
            targetBuildPath .. "/Engine"
        }


        includedirs {
            rootPath .. "/include",
            targetBuildPath .. "/External/include"
        }

    end

    dependson {
        "Engine",
        "ImGui",
        "tracy"
    }

    links {
        "Engine",
        "ImGui",
        "tracy",
        AddQuotation("SDL3"),
        AddQuotation("Shaderc"),
        AddQuotation("Slangd")
    }
