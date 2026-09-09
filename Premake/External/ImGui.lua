project "ImGui"

    kind "StaticLib"
    location(projectsPath)

    warnings "Off"

    targetdir(targetBuildPath .. "/External/lib/")
    objdir(objBuildPath .. "/%{prj.name}")

    files
    {
        rootPath .. "/External/ImGui/imgui*.cpp",
        rootPath .. "/External/ImGui/backends/imgui_impl_vulkan.cpp",
        rootPath .. "/External/ImGui/backends/imgui_impl_sdl3.cpp"
    }
    
    local vkPath = os.getenv('VULKAN_SDK')
    
    includedirs
    {
        vkPath .. "/Include/",
        rootPath .. "/External/ImGui/",
        rootPath .. "/External/ImGui/backends/",
        targetBuildPath .. "/External/include/"
    }

    local imGuiPath = targetBuildPath .. "/External/include/%{prj.name}"

    prebuildcommands {
        "{MKDIR} " .. AddQuotation(imGuiPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/ImGui") .. "/*.h" .. " " .. AddQuotation(imGuiPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/ImGui/backends/imgui_impl_vulkan.h") .. " " .. AddQuotation(imGuiPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/ImGui/backends/imgui_impl_sdl3.h") .. " " .. AddQuotation(imGuiPath)
    }
