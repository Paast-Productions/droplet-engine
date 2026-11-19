/**
 * @file WSI.cpp
 * @author Sebastian Lorensson
 * @date 2026-10-16
 * @brief Swapchain and frame pacing: BeginFrame / EndFrame around a render graph the Renderer submits
 */
#include "Integration/Vulkan/WSI/WSI.hpp"

#include "Integration/Vulkan/Core/Context.hpp"

// stl
#include <exception>
#include <print>

using namespace Vulkan;

// =====================================================================
// WSI
// =====================================================================
WSI::WSI(const Context& context)
    : m_context(&context),
      // 1. swapchain, its views and the per-image semaphores
      m_swapchain(context)
{
    // 2. one acquire semaphore per frame slot
    for (std::uint32_t frameSlot = 0; frameSlot < MAX_FRAMES_IN_FLIGHT; ++frameSlot)
        m_imageAvailableSemaphores.emplace_back(context.GetDevice(), Semaphore::Type::Binary);
}

WSI::~WSI() noexcept
{
    try
    {
        WaitIdle();
    }
    catch (const std::exception& e)
    {
        std::println(
            "Vulkan device wait during shutdown failed: {}",
            e.what());
    }
    catch (...)
    {
        std::println(
            "Vulkan device wait during shutdown failed.");
    }
}

void WSI::WaitIdle()
{
    m_context->WaitIdle();
}

std::optional<Frame> WSI::BeginFrame()
{
    // 0. nothing to draw into while minimized. SandboxApp waits for window events meanwhile.
    if (m_frameBufferMinimized)
        return std::nullopt;

    // 1. the caller has waited until the GPU finished the frame that last used this slot (FrameSubmitter::BeginFrame),
    //    so the slot's acquire semaphore is free.

    // 2. get first available swapchain image.
    //    Only an out-of-date acquire skips the frame: it did not signal the acquire semaphore, so nothing is left
    //    pending. Any other reason to recreate (a resize request, a suboptimal image) waits until after present
    //    (EndFrame), because a successful acquire has signalled the semaphore and only the frame's submit waits on
    //    (unsignals) it. Recreating here instead left it signalled, and the next acquire on it failed validation
    //    (VUID-...-01286).
    const Semaphore& imageAvailable = m_imageAvailableSemaphores[m_frameSlot];
    const AcquiredImage acquired = m_swapchain.AcquireNextImage(imageAvailable);
    if (acquired.status == SwapchainStatus::OutOfDate)
    {
        m_frameBufferResized = false;
        RecreateSwapchain();
        // Try again next frame
        return std::nullopt;
    }
    return Frame{
        .frameSlot = m_frameSlot,
        .swapchainImage = acquired.image,
        .imageAvailable = &imageAvailable,
        .acquiredSuboptimal = acquired.status == SwapchainStatus::Suboptimal
    };
}

void WSI::EndFrame(const Frame& frame)
{
    // 3. the graph has been submitted (the Renderer's FrameSubmitter): it waited on the acquire semaphore and
    //    signalled the render-finished semaphore of this image. Present waits on that.
    const SwapchainStatus presentStatus = m_swapchain.Present(frame.swapchainImage.index);

    // 4. recreate now if anything asked for it: this frame's acquire semaphore was consumed by the submit.
    if (presentStatus != SwapchainStatus::Optimal || frame.acquiredSuboptimal || m_frameBufferResized)
    {
        m_frameBufferResized = false;
        RecreateSwapchain();
    }

    // always move on to the next frame slot
    m_frameSlot = (m_frameSlot + 1) % MAX_FRAMES_IN_FLIGHT;
}

void WSI::RecreateSwapchain()
{
    // No waiting for a restore here: a busy loop on m_frameBufferMinimized never pumped window events, so the
    // restore that would end it never arrived (the window could not be un-minimized). BeginFrame returns early
    // while minimized and SandboxApp::MainLoop waits for events instead.
    WaitIdle();

    // new swapchain, views and per-image semaphores; the old swapchain is retired into the new one.
    // The generation changes, so Renderer rebuilds what depends on the swapchain (the compositor's pipeline if
    // the format changed).
    m_swapchain.Recreate();
}
