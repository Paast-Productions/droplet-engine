project "ImGui"

    kind "StaticLib"
    location(projectsPath)

    warnings "Off"

    targetdir(targetBuildPath .. "/External/lib")
    objdir(objBuildPath .. "/%{prj.name}")

    files
    {
        rootPath .. "/External/ImGui/imgui*.cpp",
        rootPath .. "/External/ImGui/backends/imgui_impl_vulkan.cpp",
        rootPath .. "/External/ImGui/backends/imgui_impl_sdl3.cpp"
    }
    
    if _TARGET_OS == "windows" then

        local vkPath = os.getenv('VULKAN_SDK')
    
        includedirs
        {
            vkPath .. "/Include/",
            rootPath .. "/External/ImGui/",
            rootPath .. "/External/ImGui/backends/",
            targetBuildPath .. "/External/include/"
        }

    else

        includedirs
        {
            rootPath .. "/External/ImGui/",
            rootPath .. "/External/ImGui/backends/",
            targetBuildPath .. "/External/include/"
        }

    end

    local imGuiPath = targetBuildPath .. "/External/include/%{prj.name}"

    prebuildcommands {
        "{MKDIR} " .. AddQuotation(imGuiPath),
        "{COPYFILE} " .. AddQuotation(rootPath .. "/External/ImGui/imgui.h") .. " " .. AddQuotation(imGuiPath),
        "{COPYFILE} " .. AddQuotation(rootPath .. "/External/ImGui/imgui_internal.h") .. " " .. AddQuotation(imGuiPath),
        "{COPYFILE} " .. AddQuotation(rootPath .. "/External/ImGui/imconfig.h") .. " " .. AddQuotation(imGuiPath),
        "{COPYFILE} " .. AddQuotation(rootPath .. "/External/ImGui/backends/imgui_impl_vulkan.h") .. " " .. AddQuotation(imGuiPath),
        "{COPYFILE} " .. AddQuotation(rootPath .. "/External/ImGui/backends/imgui_impl_sdl3.h") .. " " .. AddQuotation(imGuiPath)
    }

    buildcommands {
        "{ECHO} 'BUILDING IMGUI'"
    }
