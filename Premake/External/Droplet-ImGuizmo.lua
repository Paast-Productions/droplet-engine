project "ImGuizmo"

    kind "StaticLib"
    location(projectsPath)

    warnings "Off"

    targetdir(targetBuildPath .. "/External/lib")
    objdir(objBuildPath .. "/%{prj.name}")

    files
    {
        rootPath .. "/External/Droplet-ImGuizmo/src/*.cpp"
    }
    
    if _TARGET_OS == "windows" then

        local vkPath = os.getenv('VULKAN_SDK')
    
        includedirs
        {
            vkPath .. "/Include/",
            rootPath .. "/External/ImGui/",
            rootPath .. "/External/ImGui/backends/",
            rootPath .. "/External/Droplet-ImGuizmo/src/",
            targetBuildPath .. "/External/include/"
        }

    else

        includedirs
        {
            rootPath .. "/External/ImGui/",
            rootPath .. "/External/ImGui/backends/",
            rootPath .. "/External/Droplet-ImGuizmo/src/",
            targetBuildPath .. "/External/include/"
        }

    end
	
    dependson 
    {
        "ImGui"
    }

    local imGuizmoPath = targetBuildPath .. "/External/include/ImGuizmo"

    prebuildcommands {
        "{MKDIR} " .. AddQuotation(imGuizmoPath),
        "{COPYFILE} " .. AddQuotation(rootPath .. "/External/Droplet-ImGuizmo/src/ImGuizmo.h") .. " " .. AddQuotation(imGuizmoPath),
        "{COPYFILE} " .. AddQuotation(rootPath .. "/External/Droplet-ImGuizmo/src/ImCurveEdit.h") .. " " .. AddQuotation(imGuizmoPath),
        "{COPYFILE} " .. AddQuotation(rootPath .. "/External/Droplet-ImGuizmo/src/ImGradient.h") .. " " .. AddQuotation(imGuizmoPath),
        "{COPYFILE} " .. AddQuotation(rootPath .. "/External/Droplet-ImGuizmo/src/ImLightRig.h") .. " " .. AddQuotation(imGuizmoPath),
        "{COPYFILE} " .. AddQuotation(rootPath .. "/External/Droplet-ImGuizmo/src/ImSequencer.h") .. " " .. AddQuotation(imGuizmoPath),
        "{COPYFILE} " .. AddQuotation(rootPath .. "/External/Droplet-ImGuizmo/src/ImVectorEditor.h") .. " " .. AddQuotation(imGuizmoPath),
        "{COPYFILE} " .. AddQuotation(rootPath .. "/External/Droplet-ImGuizmo/src/ImZoomSlider.h") .. " " .. AddQuotation(imGuizmoPath)
    }

    buildcommands {
        "{ECHO} 'BUILDING IMGUIZMO'"
    }
