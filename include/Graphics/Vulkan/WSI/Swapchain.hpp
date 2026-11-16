/**
 * @file Swapchain.hpp
 * @brief the VkSwapchainKHR with its images, views and per-image semaphores; chooses its settings
 *        and recreates it.
 */
#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "Vulkan/Abstraction/Semaphore.hpp"

namespace Vulkan
{
    class Context;

    /// @brief How usable the swapchain still is, from an acquire or a present.
    enum class SwapchainStatus
    {
        Optimal,    ///< Matches the surface.
        Suboptimal, ///< Still usable, but should be recreated (after this frame's present).
        OutOfDate   ///< Unusable; must be recreated before the next acquire.
    };

    inline const char* ToString(SwapchainStatus status)
    {
        switch (status)
        {
        case SwapchainStatus::Optimal: return "Optimal";
        case SwapchainStatus::Suboptimal: return "Suboptimal";
        case SwapchainStatus::OutOfDate: return "OutOfDate";
        default:
            return "Unknown";
        }
    }


    /// @brief One swapchain image, as the frame rendering into it sees it.
    /// @details Non-owning: the swapchain owns the image, its view and its semaphore; valid until the swapchain is
    ///          recreated. The image's old contents are undefined after every acquire.
    struct SwapchainImage
    {
        std::uint32_t index{0};                     ///< Index into the swapchain's images.
        vk::Image image{};                          ///< The image.
        vk::ImageView view{};                       ///< Colour attachment view of the image.
        vk::Extent2D extent{};                      ///< Size in pixels.
        vk::Format format{vk::Format::eUndefined}; ///< Pixel format.
        const Semaphore* renderFinished{nullptr};  ///< Binary; signalled by the frame's last use, waited on by present.
    };

    /// @brief An acquired image, or the reason there is none.
    struct AcquiredImage
    {
        SwapchainStatus status{SwapchainStatus::Optimal}; ///< OutOfDate: no image was acquired.
        SwapchainImage image{};                           ///< Valid unless status is OutOfDate.
    };

    /// @brief The swapchain: chooses format (B8G8R8A8 sRGB), present mode (mailbox, else FIFO), image count (at
    ///        least 3) and extent; owns the VkSwapchainKHR, a view per image and one render-finished semaphore per
    ///        image (present keeps it busy until the image is acquired again). Recreate() increments
    ///        GetGeneration(). Not copyable or movable: the views refer to its images.
    class Swapchain
    {
    public:
        /**
         * @brief Creates the first swapchain for the context's surface.
         * @param context device, surface and window system; must outlive the swapchain
         * @throws std::runtime_error if the window is 0 x 0
         * @throws vk::SystemError if creation fails
         */
        explicit Swapchain(const Context& context);
        ~Swapchain() = default;

        // no copy, no move
        Swapchain(const Swapchain&) = delete;
        Swapchain& operator=(const Swapchain&) = delete;
        Swapchain(Swapchain&&) = delete;
        Swapchain& operator=(Swapchain&&) = delete;

        /**
         * @brief Acquires the next image to render into.
         * @param imageAvailable binary semaphore signalled when the image can be written; left unsignalled when the
         *                       status is OutOfDate
         * @return the image, and whether the swapchain should be recreated
         * @throws vk::SystemError or std::runtime_error for any error other than out of date
         */
        [[nodiscard]] AcquiredImage AcquireNextImage(const Semaphore& imageAvailable) const;

        /**
         * @brief Presents image `imageIndex` on the context's graphics queue, waiting on its render-finished semaphore.
         * @pre the frame's submit signals GetRenderFinishedSemaphore(imageIndex)
         * @return whether the swapchain should be recreated
         * @throws vk::SystemError or std::runtime_error for any error other than suboptimal or out of date
         */
        SwapchainStatus Present(std::uint32_t imageIndex) const;

        /**
         * @brief Replaces the swapchain (handing the old one to the driver) and everything per image.
         * @pre the GPU is idle: nothing may still use the old images, views or semaphores
         * @throws std::runtime_error if the window is 0 x 0
         */
        void Recreate();

        /// @brief Number of swapchain images (at least the requested minimum; the driver may create more).
        [[nodiscard]] std::uint32_t GetImageCount() const noexcept
        {
            return static_cast<std::uint32_t>(m_images.size());
        }

        /// @brief Swapchain image `imageIndex`; owned by the swapchain.
        [[nodiscard]] vk::Image GetImage(std::uint32_t imageIndex) const { return m_images.at(imageIndex); }

        /// @brief View of swapchain image `imageIndex`.
        [[nodiscard]] vk::ImageView GetImageView(std::uint32_t imageIndex) const
        {
            return *m_imageViews.at(imageIndex);
        }

        /// @brief The semaphore the frame rendering image `imageIndex` signals and present waits on.
        [[nodiscard]] const Semaphore& GetRenderFinishedSemaphore(std::uint32_t imageIndex) const
        {
            return m_renderFinishedSemaphores.at(imageIndex);
        }

        /// @brief Image format.
        [[nodiscard]] vk::Format GetFormat() const noexcept { return m_surfaceFormat.format; }

        /// @brief Image format and colour space.
        [[nodiscard]] vk::SurfaceFormatKHR GetSurfaceFormat() const noexcept { return m_surfaceFormat; }

        /// @brief Image size in pixels.
        [[nodiscard]] vk::Extent2D GetExtent() const noexcept { return m_extent; }

        /// @brief Image `imageIndex` with its view, size, format and render-finished semaphore.
        [[nodiscard]] SwapchainImage GetSwapchainImage(std::uint32_t imageIndex) const;

        /// @brief The present mode it was created with.
        [[nodiscard]] vk::PresentModeKHR GetPresentMode() const noexcept { return m_presentMode; }

        /// @brief Increments on every (re)creation; the first swapchain is generation 1.
        [[nodiscard]] std::uint64_t GetGeneration() const noexcept { return m_generation; }

    private:
        /// @brief Chooses the settings, creates the swapchain (replacing the current one, if any), then a view and a
        ///        render-finished semaphore per image.
        void Create();

        /// @brief Creates one 2D colour view per image.
        void CreateImageViews();

        /// @brief (Re)creates one render-finished semaphore per swapchain image.
        void CreateRenderFinishedSemaphores();

        // helper functions
        const vk::raii::Device& Device() const noexcept;

    private:
        const Context* m_context; ///< Not owned; outlives the swapchain.

        vk::SurfaceFormatKHR m_surfaceFormat{};
        vk::Extent2D m_extent{};
        vk::PresentModeKHR m_presentMode{vk::PresentModeKHR::eFifo};

        // dependency order top to bottom
        vk::raii::SwapchainKHR m_swapchain{nullptr};       ///< Never null after construction.
        std::vector<vk::Image> m_images;                   ///< Non-owning: the swapchain owns its images.
        std::vector<vk::raii::ImageView> m_imageViews;     ///< One per image, destroyed before the swapchain.
        std::vector<Semaphore> m_renderFinishedSemaphores; ///< Binary, one per swapchain image.
        std::uint64_t m_generation{0};                     ///< See GetGeneration().
    };
}
