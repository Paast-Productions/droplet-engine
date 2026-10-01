project "tracy"
    kind "StaticLib"

    location(projectsPath)

    warnings "Off"

    targetdir(targetBuildPath .. "/External/lib/")
    objdir(objBuildPath .. "/%{prj.name}")
    
    local tracyPath = targetBuildPath .. "/External/include/%{prj.name}"
    local tracyServerPath = tracyPath .. "/server"
    local tracyPublicPath = tracyPath .. "/public"
    local tracyClientPath = tracyPublicPath .. "/client"
    local tracyCommonPath = tracyPublicPath .. "/common"
    local tracyLibBacktracePath = tracyPublicPath .. "/libbacktrace"
    local tracyTracyPath = tracyPublicPath .. "/tracy"
    
    local vkPath = os.getenv('VULKAN_SDK')

	defines{ "TRACY_ENABLE", "TRACY_ON_DEMAND" }
	
	filter {"system:windows"}
		buildoptions {"/Zi"}
	
	filter {}
	
    files 
	{
        rootPath .. "/External/Tracy/public/TracyClient.cpp",
    }
	
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
        "{MKDIR} " .. AddQuotation(tracyServerPath),
        "{MKDIR} " .. AddQuotation(tracyPublicPath),
        "{MKDIR} " .. AddQuotation(tracyClientPath),
        "{MKDIR} " .. AddQuotation(tracyCommonPath),
        "{MKDIR} " .. AddQuotation(tracyLibBacktracePath),
        "{MKDIR} " .. AddQuotation(tracyTracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/tracy_pdqsort.h") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/tracy_robin_hood.h") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/tracy_xxhash.h") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyBroadcast.hpp") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyCharUtil.hpp") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyEvent.hpp") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyFileHeader.hpp") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyFileMeta.hpp") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyFileRead.hpp") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyFileWrite.hpp") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyMemory.hpp") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyMmap.hpp") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyPopcnt.hpp") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyPrint.hpp") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyShortPtr.hpp") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracySlab.hpp") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracySort.hpp") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracySortedVector.hpp") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyStringDiscovery.hpp") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracySysUtil.hpp") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyTaskDispatch.hpp") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyTextureCompression.hpp") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyThreadCompress.hpp") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyVarArray.hpp") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyVector.hpp") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/server/TracyWorker.hpp") .. " " .. AddQuotation(tracyServerPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/windows/TracyETW_compat.h") .. " " .. AddQuotation(tracyClientPath .. "/windows"),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/tracy_concurrentqueue.h") .. " " .. AddQuotation(tracyClientPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/tracy_SPSCQueue.h") .. " " .. AddQuotation(tracyClientPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyCallstack.h") .. " " .. AddQuotation(tracyClientPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/tracy_rpmalloc.hpp") .. " " .. AddQuotation(tracyClientPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyArmCpuTable.hpp") .. " " .. AddQuotation(tracyClientPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyCallstack.hpp") .. " " .. AddQuotation(tracyClientPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyCpuid.hpp") .. " " .. AddQuotation(tracyClientPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyDebug.hpp") .. " " .. AddQuotation(tracyClientPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyDxt1.hpp") .. " " .. AddQuotation(tracyClientPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyElf.hpp") .. " " .. AddQuotation(tracyClientPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyFastVector.hpp") .. " " .. AddQuotation(tracyClientPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyKCore.hpp") .. " " .. AddQuotation(tracyClientPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyLock.hpp") .. " " .. AddQuotation(tracyClientPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyMangle.hpp") .. " " .. AddQuotation(tracyClientPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyProfiler.hpp") .. " " .. AddQuotation(tracyClientPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyRingBuffer.hpp") .. " " .. AddQuotation(tracyClientPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyScoped.hpp") .. " " .. AddQuotation(tracyClientPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyStringHelpers.hpp") .. " " .. AddQuotation(tracyClientPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracySysPower.hpp") .. " " .. AddQuotation(tracyClientPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracySysTime.hpp") .. " " .. AddQuotation(tracyClientPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracySysTrace.hpp") .. " " .. AddQuotation(tracyClientPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/client/TracyThread.hpp") .. " " .. AddQuotation(tracyClientPath),		
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyApi.h") .. " " .. AddQuotation(tracyCommonPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyFormat.h") .. " " .. AddQuotation(tracyCommonPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/tracy_lz4.hpp") .. " " .. AddQuotation(tracyCommonPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/tracy_lz4hc.hpp") .. " " .. AddQuotation(tracyCommonPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyAlign.hpp") .. " " .. AddQuotation(tracyCommonPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyAlloc.hpp") .. " " .. AddQuotation(tracyCommonPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyAssert.hpp") .. " " .. AddQuotation(tracyCommonPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyColor.hpp") .. " " .. AddQuotation(tracyCommonPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyForceInline.hpp") .. " " .. AddQuotation(tracyCommonPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyMutex.hpp") .. " " .. AddQuotation(tracyCommonPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyProtocol.hpp") .. " " .. AddQuotation(tracyCommonPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyQueue.hpp") .. " " .. AddQuotation(tracyCommonPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracySocket.hpp") .. " " .. AddQuotation(tracyCommonPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyStackFrames.hpp") .. " " .. AddQuotation(tracyCommonPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyString.hpp") .. " " .. AddQuotation(tracyCommonPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracySystem.hpp") .. " " .. AddQuotation(tracyCommonPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyTaggedUserlandAddress.hpp") .. " " .. AddQuotation(tracyCommonPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyVersion.hpp") .. " " .. AddQuotation(tracyCommonPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyWinFamily.hpp") .. " " .. AddQuotation(tracyCommonPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/common/TracyYield.hpp") .. " " .. AddQuotation(tracyCommonPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/libbacktrace/backtrace.hpp") .. " " .. AddQuotation(tracyLibBacktracePath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/libbacktrace/config.h") .. " " .. AddQuotation(tracyLibBacktracePath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/libbacktrace/filenames.hpp") .. " " .. AddQuotation(tracyLibBacktracePath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/libbacktrace/internal.hpp") .. " " .. AddQuotation(tracyLibBacktracePath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/tracy/Tracy.hpp") .. " " .. AddQuotation(tracyTracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/tracy/TracyLua.hpp") .. " " .. AddQuotation(tracyTracyPath),
        "{COPY} " .. AddQuotation(rootPath .. "/External/tracy/public/tracy/TracyVulkan.hpp") .. " " .. AddQuotation(tracyTracyPath)
    }

    buildcommands {
        "{ECHO} 'BUILDING TRACY'"
    }