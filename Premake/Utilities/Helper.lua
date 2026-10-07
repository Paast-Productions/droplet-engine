function AddQuotation(str)
    return "\"" .. str .. "\""
end

function UseDefaultPCH()
    pchheader "PCH.h"
    pchsource(srcPath .. "/PCH/PCH.cpp")
    
    forceincludes { "PCH.h" }
    
    includedirs { rootPath .. "/include/PCH" }
    files { srcPath .. "/PCH/PCH.cpp" }
end

function UseGraphicsPCH()
    pchheader "GraphicsPCH.h"
    pchsource(srcPath .. "/PCH/GraphicsPCH.cpp")
    
    forceincludes { "GraphicsPCH.h" }
    
    includedirs { rootPath .. "/include/PCH" }
    files {
        srcPath .. "/PCH/GraphicsPCH.cpp"
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