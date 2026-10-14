function AddQuotation(str)
    return "\"" .. str .. "\""
end

function UseDefaultPCH()
    pchheader "PCH.h"
    pchsource(srcPath .. "/PCH/PCH.cpp")
    
    forceincludes { "PCH.h" }
    
    includedirs { rootPath .. "/include/PCH" }
    files { srcPath .. "/PCH/PCH.cpp" }
    
    defines { "GLM_FORCE_DEPTH_ZERO_TO_ONE" }
end

function UseGraphicsPCH()
    pchheader "GraphicsPCH.h"
    pchsource(srcPath .. "/PCH/GraphicsPCH.cpp")
    
    forceincludes { "GraphicsPCH.h" }
    
    includedirs { rootPath .. "/include/PCH" }
    files {
        srcPath .. "/PCH/GraphicsPCH.cpp"
    }
    
    defines {
        "GLM_FORCE_DEPTH_ZERO_TO_ONE",
        "GLM_ENABLE_EXPERIMENTAL",
        "VULKAN_HPP_NO_STRUCT_CONSTRUCTORS",
        "VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS",
        "IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING"
    }
end

function UseCustomPCH(pchName)

    local headerName = pchName .. ".h"
    local sourceName = pchName .. ".cpp"
    
    pchheader(pchheader)
    pchsource(srcPath .. "/" .. sourceName)
    
    forceincludes { headerName }

    includedirs { rootPath .. "/include/PCH" }
    files {
        srcPath .. "/PCH/" .. sourceName
    }
end