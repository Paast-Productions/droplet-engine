#pragma once
#include <cstdint>

#include "Integration/Vulkan/Core/ImageState.hpp"
#include "Integration/Vulkan/Abstraction/Buffer.hpp"
#include "Integration/Vulkan/Abstraction/Image.hpp"

namespace Vulkan
{
    class Context;

    /// Blocking staging uploader of Images and Buffers
    class Uploader
    {
    public:
        explicit Uploader(const Context& context);

        /// Device-local buffer filled from `data` through a staging buffer.
        Buffer CreateDeviceBuffer(const void* data, vk::DeviceSize size, vk::BufferUsageFlags usage);

        /// @brief Sampled 2D image filled from tightly packed pixels, its mip levels built by blits from level 0;
        ///        returned in ShaderReadOnly layout (fragment shader), the upload finished.
        /// @param mipLevels levels wanted; 1 if the format cannot be blitted with linear filtering (Image::MipLevels
        ///                  says how many were made)
        Image UploadImage2D(const void* pixels,
            vk::DeviceSize byteSize,
            vk::Extent2D extent,
            vk::Format format,
            std::uint32_t mipLevels);

        /// @brief Clears a colour image to opaque black and leaves it ShaderReadOnly (fragment shader), blocking.
        /// @param image must have transfer-destination usage; its old contents are discarded
        void ClearToSampled(vk::Image image, const vk::ImageSubresourceRange& range);

    private:
        /**
         * @brief Create staging buffer
         * @param data data to fill buffer with
         * @param size size of the data
         * @return created staging buffer
         */
        Buffer CreateStaging(const void* data, vk::DeviceSize size) const;

        /// Blocking submit
        template <class Record>
        void SubmitAndWait(Record&& record);

        const Context* m_context; ///< Not owned; outlives this.
        vk::raii::CommandPool m_pool;
        vk::raii::Fence m_fence;
    };
}
