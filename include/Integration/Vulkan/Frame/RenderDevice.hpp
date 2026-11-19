/**
 * @file RenderDevice.hpp
 * @brief Vulkan::RenderDevice: frame submission and the lifetime of GPU resources, under the Renderer.
 */
#pragma once

#include <cstdint>
#include <functional>
#include <utility>
#include <vector>

#include "Integration/Assets/AssetTransfer.hpp"
#include "Integration/Vulkan/Core/QueueTimelineValues.hpp"
#include "Integration/Vulkan/Frame/FrameSubmitter.hpp"
#include "Integration/Vulkan/Resources/DeletionQueue.hpp"
#include "Integration/Vulkan/Resources/GpuResourceManager.hpp"
#include "Integration/Vulkan/Resources/PipelineLibrary.hpp"

namespace Vulkan
{
    class Context;

    /**
     * @brief Owns the FrameSubmitter, the GPU resources (GpuResourceManager, ShaderLibrary) and the DeletionQueue, and
     *        keeps their lifetimes: the job Granite's Device does.
     * @details Per frame: BeginFrame (resource use tracking), the Renderer resolves assets (Prepare), WaitForFrameSlot
     * before the WSI acquires, the graph is submitted through Submitter(), EndFrame (evict idle resources, tell the
     * listeners, destroy what the GPU is done with). Anything that caches GPU handles registers a listener with
     * OnResourcesReleased. Resident() is what the Renderer tells the asset provider the GPU holds.
     */
    class RenderDevice
    {
    public:
        /// @brief Called after resources were evicted or replaced; drop what refers to stale handles. Gets the frame's
        ///        retire values (anything still bound by this frame must live until the GPU is past them).
        using ReleaseListener = std::function<void(const QueueTimelineValues& frameRetireValues)>;

        /**
         * @brief Creates the frame submitter (one timeline per queue), the deletion queue, the resource manager
         *        (with its placeholders) and the shader library.
         * @param context        must outlive the device
         * @param framesInFlight frame slots of the frame submitter (WSI::FramesInFlight())
         */
        RenderDevice(const Context& context, std::uint32_t framesInFlight);

        // no copy no move
        RenderDevice(const RenderDevice&) = delete;
        RenderDevice& operator=(const RenderDevice&) = delete;
        RenderDevice(RenderDevice&&) = delete;
        RenderDevice& operator=(RenderDevice&&) = delete;

        /// @brief Starts a frame: resources resolved from here on count as used.
        void BeginFrame();

        /// @brief Waits until the GPU has finished the frame that last used `frameSlot` on every queue, and frees its
        ///        command buffers; its acquire semaphore is free afterwards.
        void WaitForFrameSlot(std::uint32_t frameSlot) { m_frameSubmitter.BeginFrame(frameSlot); }

        /// @brief Ends a frame (also one that was skipped): evicts idle resources, tells the listeners if anything was
        ///        released, and destroys what every queue's timeline has passed.
        void EndFrame();

        /// @brief Registers `listener`; called by EndFrame after resources were evicted or replaced.
        void OnResourcesReleased(ReleaseListener listener) { m_releaseListeners.push_back(std::move(listener)); }

        /// @brief Destroys `object` once the GPU is past `retireValues` on every queue.
        template <class T>
        void Retire(T&& object, const QueueTimelineValues& retireValues)
        {
            m_deletionQueue.Retire(std::forward<T>(object), retireValues);
        }

        /// @brief Destroys everything waiting for the GPU. @pre the GPU is idle
        void FlushDeletions() { m_deletionQueue.Flush(); }

        /// @brief Uploads meshes and textures (a newer revision replaces the resident one) and compiles shaders.
        /// @details Not part of a frame: call it between frames. The spans only have to live until it returns.
        void Upload(const Assets::AssetUploads& uploads);

        /// @brief What the GPU holds, with the revisions it was uploaded from.
        [[nodiscard]] Assets::ResidentAssets Resident() const;

        /// @brief Records and submits render graphs; one timeline per queue.
        [[nodiscard]] FrameSubmitter& Submitter() noexcept { return m_frameSubmitter; }
        /// @brief Meshes, textures, render targets and histories, with their placeholders.
        [[nodiscard]] GpuResourceManager& Resources() noexcept { return m_resources; }
        [[nodiscard]] const GpuResourceManager& Resources() const noexcept { return m_resources; }
        /// @brief Scene shaders by asset GUID, with the default shader.
        [[nodiscard]] ShaderLibrary& Shaders() noexcept { return m_shaders; }
        /// @brief Evicted resources until the GPU is past their last use (for listeners that retire their own objects).
        [[nodiscard]] DeletionQueue& Deletions() noexcept { return m_deletionQueue; }

    private:
        // dependency order top to bottom
        FrameSubmitter m_frameSubmitter;  ///< Command buffers and submits per graph submission, timeline per queue
        DeletionQueue m_deletionQueue;    ///< Destroyed after the resources that retire into it
        GpuResourceManager m_resources;   ///< Retires evicted and replaced resources into m_deletionQueue
        ShaderLibrary m_shaders;          ///< Scene shaders; owns the Slang compiler
        std::vector<ReleaseListener> m_releaseListeners;
    };
}
