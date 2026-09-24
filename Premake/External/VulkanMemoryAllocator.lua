project "VulkanMemoryAllocator"
    kind "Utility"
    location(projectsPath)

    warnings "Off"

    targetdir(targetBuildPath .. "/External/lib")
    objdir(objBuildPath .. "/%{prj.name}")
    
    local vulkanMemoryAllocatorPath = targetBuildPath .. "/External/include"

    prebuildcommands
    {
        "{MKDIR} " .. AddQuotation(vulkanMemoryAllocatorPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/VulkanMemoryAllocator/include/vk_mem_alloc.hpp") .. " " .. AddQuotation(vulkanMemoryAllocatorPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/VulkanMemoryAllocator/include/vk_mem_alloc_enums.hpp") .. " " .. AddQuotation(vulkanMemoryAllocatorPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/VulkanMemoryAllocator/include/vk_mem_alloc_funcs.hpp") .. " " .. AddQuotation(vulkanMemoryAllocatorPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/VulkanMemoryAllocator/include/vk_mem_alloc_handles.hpp") .. " " .. AddQuotation(vulkanMemoryAllocatorPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/VulkanMemoryAllocator/include/vk_mem_alloc_imported.hpp") .. " " .. AddQuotation(vulkanMemoryAllocatorPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/VulkanMemoryAllocator/include/vk_mem_alloc_raii.hpp") .. " " .. AddQuotation(vulkanMemoryAllocatorPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/VulkanMemoryAllocator/include/vk_mem_alloc_static_assertions.hpp") .. " " .. AddQuotation(vulkanMemoryAllocatorPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/VulkanMemoryAllocator/include/vk_mem_alloc_structs.hpp") .. " " .. AddQuotation(vulkanMemoryAllocatorPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/VulkanMemoryAllocator/include/vk_mem_alloc_to_string.hpp") .. " " .. AddQuotation(vulkanMemoryAllocatorPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/VulkanMemoryAllocator/VulkanMemoryAllocator/include/vk_mem_alloc.h") .. " " .. AddQuotation(vulkanMemoryAllocatorPath)
    }