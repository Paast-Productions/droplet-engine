project "ImGuizmo"

    kind "StaticLib"
    location(projectsPath)

    warnings "Off"

    targetdir(targetBuildPath .. "/External/lib")
    objdir(objBuildPath .. "/%{prj.name}")

    files
    {
        rootPath .. "/External/ImGuizmo/src/*.cpp"
    }
    
    if _TARGET_OS == "windows" then

        local vkPath = os.getenv('VULKAN_SDK')
    
        includedirs
        {
            vkPath .. "/Include/",
            rootPath .. "/External/ImGui/",
            rootPath .. "/External/ImGui/backends/",
            rootPath .. "/External/ImGuizmo/src/",
            targetBuildPath .. "/External/include/"
        }

    else

        includedirs
        {
            rootPath .. "/External/ImGui/",
            rootPath .. "/External/ImGui/backends/",
            rootPath .. "/External/ImGuizmo/src/",
            targetBuildPath .. "/External/include/"
        }

    end
	
    dependson 
    {
        "ImGui"
    }

    local imGuizmoPath = targetBuildPath .. "/External/include/%{prj.name}"

    prebuildcommands {
        "{MKDIR} " .. AddQuotation(imGuizmoPath),
        "{COPYFILE} " .. AddQuotation(rootPath .. "/External/ImGuizmo/src/ImGuizmo.h") .. " " .. AddQuotation(imGuizmoPath),
        "{COPYFILE} " .. AddQuotation(rootPath .. "/External/ImGuizmo/src/ImCurveEdit.h") .. " " .. AddQuotation(imGuizmoPath),
        "{COPYFILE} " .. AddQuotation(rootPath .. "/External/ImGuizmo/src/ImGradient.h") .. " " .. AddQuotation(imGuizmoPath),
        "{COPYFILE} " .. AddQuotation(rootPath .. "/External/ImGuizmo/src/ImLightRig.h") .. " " .. AddQuotation(imGuizmoPath),
        "{COPYFILE} " .. AddQuotation(rootPath .. "/External/ImGuizmo/src/ImSequencer.h") .. " " .. AddQuotation(imGuizmoPath),
        "{COPYFILE} " .. AddQuotation(rootPath .. "/External/ImGuizmo/src/ImVectorEditor.h") .. " " .. AddQuotation(imGuizmoPath),
        "{COPYFILE} " .. AddQuotation(rootPath .. "/External/ImGuizmo/src/ImZoomSlider.h") .. " " .. AddQuotation(imGuizmoPath)
    }

    buildcommands {
        "{ECHO} 'BUILDING IMGUIZMO'"
    }
