/**
 * @file WSI.hpp
 * @brief WSI class declaration
 * @author Sebastian Lorensson
 */
#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>

#include <cstdint>
#include <optional>
#include <vector>

#include "Graphics/Vulkan/WSI/Swapchain.hpp"

namespace Droplet::Graphics::Vulkan
{
    class Context;

    /**
     * @brief Describes a frame between WSI::BeginFrame and WSI::EndFrame
     *
     * @details Execute the frame's render graph with WSI::GetFrameSubmitter() (GraphExecutor does), handing it the two
     * semaphores below for the swapchain image's external wait and signal, then hand the frame back to EndFrame.
     * After EndFrame Frame becomes invalid
     * Hold raw image and imageview because swapchain owns the image
     */
    struct Frame
    {
        std::uint32_t frameSlot{0};               ///< Frame slot, in [0, WSI::FramesInFlight()).
        SwapchainImage swapchainImage{};          ///< The acquired image; its old contents are undefined.
        const Semaphore* imageAvailable{nullptr}; ///< Binary; signalled by the acquire, waited on by the graph's first use.
        bool acquiredSuboptimal{false};           ///< The swapchain is recreated after this frame's present.
    };

    /**
     * @brief Window system integration (WSI): the swapchain and frame pacing.
     * @details Owns the Swapchain and the per-slot acquire semaphores, and runs the frame API. Per frame: the caller
     * waits until the GPU is done with FrameSlot() (the Renderer's FrameSubmitter), BeginFrame (acquire), the caller
     * executes the frame's render graph, EndFrame (present, recreate the swapchain if needed, next slot).
     * Whoever caches anything that depends on the swapchain compares SwapchainGeneration() after BeginFrame and
     * EndFrame. Driven by Vulkan::Renderer; the Context is owned there.
     */
    class WSI
    {
    public:
        /**
         * @brief Creates the swapchain and the frame slots' acquire semaphores.
         * @param context device, queue and surface; must outlive the WSI
         */
        explicit WSI(const Context& context);

        /// @waits for gpu to go idle before destroying
        ~WSI() noexcept;

        // no copy no move
        WSI(const WSI&) = delete;
        WSI& operator=(const WSI&) = delete;
        WSI(WSI&&) = delete;
        WSI& operator=(WSI&&) = delete;

        /// @brief Blocks until GPU has finished its work
        void WaitIdle();

        /**
         * @brief Starts a frame in slot FrameSlot(): acquires a swapchain image.
         * @details Returns nothing while minimized, or when the swapchain was out of date (it is recreated; skip this
         *          frame; the slot stays the same).
         * @pre the GPU has finished the frame that last used FrameSlot() on every queue, so the slot's acquire
         *      semaphore is free (FrameSubmitter::BeginFrame(FrameSlot()) waits for that)
         * @return the frame; execute exactly one render graph for it, then pass it to EndFrame exactly once
         * @throws vk::SystemError or std::runtime_error if the wait or the acquire fails
         */
        [[nodiscard]] std::optional<Frame> BeginFrame();

        /**
         * @brief Finishes a frame: presents, and recreates the swapchain if it was resized, suboptimal or out of date.
         * @param frame what BeginFrame returned
         * @pre the frame's graph has been submitted (it waits on frame.imageAvailable, signals
         *      frame.swapchainImage.renderFinished and leaves the swapchain image in Present)
         * @throws vk::SystemError or std::runtime_error if the present fails
         */
        void EndFrame(const Frame& frame);

        /// @brief The slot the next BeginFrame uses; EndFrame moves to the next one.
        [[nodiscard]] std::uint32_t FrameSlot() const noexcept { return m_frameSlot; }

        /// When window is resized, ask to resize swapchain after next present
        void OnResize() { m_frameBufferResized = true; }
        void OnRestore() { m_frameBufferMinimized = false; }
        void OnMinimize() { m_frameBufferMinimized = true; }

        /// @brief True between OnMinimize and OnRestore; BeginFrame returns nothing then.
        [[nodiscard]] bool IsMinimized() const noexcept { return m_frameBufferMinimized; }

        /// @brief Current swapchain format.
        vk::Format SwapchainFormat() const noexcept { return m_swapchain.GetFormat(); }

        /// @brief Current swapchain size in pixels (what SwapchainRelative transients scale).
        vk::Extent2D SwapchainExtent() const noexcept { return m_swapchain.GetExtent(); }

        /// @brief Increments whenever the swapchain is recreated; compare it to rebuild what depends on the swapchain.
        [[nodiscard]] std::uint64_t SwapchainGeneration() const noexcept { return m_swapchain.GetGeneration(); }

        /// @brief How many frames the GPU may be working on at once.
        static constexpr std::uint32_t FramesInFlight() noexcept { return MAX_FRAMES_IN_FLIGHT; }

    private:
        // swapchain utils
        /// Waits for the GPU and recreates the swapchain
        void RecreateSwapchain();

    private:
        static constexpr std::uint32_t MAX_FRAMES_IN_FLIGHT{2};

        const Context* m_context; ///< Not owned; outlives the WSI (owned by Renderer)

        // == Swapchain ==
        Swapchain m_swapchain; ///< Images, views and render-finished semaphores

        // == Frame synchronisation ==
        std::vector<Semaphore> m_imageAvailableSemaphores;           ///< Binary acquire semaphores, 1 per frame slot
        std::uint32_t m_frameSlot{0};                                ///< Current frame slot

        /// flags
        bool m_frameBufferResized{false};
        bool m_frameBufferMinimized{false};
    };
}
