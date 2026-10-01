project "tracy"

    kind "StaticLib"
    location(projectsPath)

    warnings "Off"

    targetdir(targetBuildPath .. "/External/lib/")
    objdir(objBuildPath .. "/%{prj.name}")
    
    local tracyPath = targetBuildPath .. "/External/include/%{prj.name}"
    
    local vkPath = os.getenv('VULKAN_SDK')
	
    files 
	{
        rootPath .. "/External/Tracy/public/TracyClient.cpp",
    }

	defines{ "TRACY_ENABLE", "TRACY_DETAILED" }
	
	filter {"system:windows"}
		buildoptions {"/Zi"}
	
	filter {}
	
    includedirs
	{
        vkPath .. "/Include/",
        rootPath .. "/Tracy/server/",
        rootPath .. "/Tracy/public/",
        rootPath .. "/Tracy/public/client/",
        rootPath .. "/Tracy/public/common/",
        rootPath .. "/Tracy/public/libbacktrace/",
        rootPath .. "/Tracy/public/tracy/",
        targetBuildPath .. "/External/include/"
    }

    prebuildcommands {
        "{MKDIR} " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/tracy_pdqsort.h") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/tracy_robin_hood.h") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/tracy_xxhash.h") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyBroadcast.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyCharUtil.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyEvent.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyFileHeader.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyFileMeta.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyFileRead.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyFileWrite.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyMemory.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyMmap.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyPopcnt.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyPrint.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyShortPtr.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracySlab.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracySort.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracySortedVector.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyStringDiscovery.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracySysUtil.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyTaskDispatch.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyTextureCompression.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyThreadCompress.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyVarArray.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyVector.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyWorker.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/windows/TracyETW_compat.h") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/tracy_concurrentqueue.h") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/tracy_SPSCQueue.h") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyCallstack.h") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/tracy_rpmalloc.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyArmCpuTable.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyCallstack.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyCpuid.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyDebug.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyDxt1.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyElf.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyFastVector.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyKCore.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyLock.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyMangle.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyProfiler.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyRingBuffer.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyScoped.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyStringHelpers.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracySysPower.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracySysTime.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracySysTrace.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyThread.hpp") .. " " .. AddQuotation(tracyPath),		
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyApi.h") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyFormat.h") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/tracy_lz4.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/tracy_lz4hc.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyAlign.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyAlloc.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyAssert.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyColor.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyForceInline.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyMutex.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyProtocol.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyQueue.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracySocket.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyStackFrames.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyString.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracySystem.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyTaggedUserlandAddress.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyVersion.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyWinFamily.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyYield.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/libbacktrace/backtrace.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/libbacktrace/config.h") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/libbacktrace/filenames.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/libbacktrace/internal.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/tracy/Tracy.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/tracy/TracyLua.hpp") .. " " .. AddQuotation(tracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/tracy/TracyVulkan.hpp") .. " " .. AddQuotation(tracyPath)
    }

    buildcommands {
        "{ECHO} 'BUILDING TRACY'"
    }