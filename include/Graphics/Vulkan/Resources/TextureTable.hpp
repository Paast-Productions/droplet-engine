/**
 * @file TextureTable.hpp
 * @brief The texture array shaders index with PushConstantData::textureID (set 2)
 */
#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "Graphics/Vulkan/Core/QueueTimelineValues.hpp"

namespace Droplet::Graphics::Vulkan
{
    class Context;
    class DeletionQueue;
    class TextureTable;

    /// @brief An index in the texture table, freed when the slot is destroyed (retire it with its image)
    class TextureSlot
    {
    public:
        TextureSlot() = default;
        TextureSlot(TextureTable& table, std::uint32_t index) : m_table(&table), m_index(index) {}
        ~TextureSlot();

        // no copy
        TextureSlot(const TextureSlot&) = delete;
        TextureSlot& operator=(const TextureSlot&) = delete;
        TextureSlot(TextureSlot&& other) noexcept
            : m_table(std::exchange(other.m_table, nullptr)), m_index(other.m_index) {}
        TextureSlot& operator=(TextureSlot&& other) noexcept;

        /// @brief The index shaders sample with; 0 for an empty slot
        [[nodiscard]] std::uint32_t Index() const noexcept { return m_index; }

    private:
        TextureTable* m_table = nullptr;
        std::uint32_t m_index = 0;
    };

    /**
     * @brief One array of combined image samplers for every texture, render target and history
     * @details The array grows when it is full (a larger variable descriptor count; pipelines are unaffected). With
     *          USE_BINDLESS one set is updated at any time; otherwise each frame slot has a set, brought up to date by
     *          BeginFrame once the GPU is done with that slot
     */
    class TextureTable
    {
    public:
        /**
         * @brief Creates the layout and the first sets
         * @param context The device; must outlive the table
         * @param framesInFlight The number of frame slots
         */
        TextureTable(const Context& context, std::uint32_t framesInFlight);
        ~TextureTable();

        // no copy no move
        TextureTable(const TextureTable&) = delete;
        TextureTable& operator=(const TextureTable&) = delete;
        TextureTable(TextureTable&&) = delete;
        TextureTable& operator=(TextureTable&&) = delete;

        /**
         * @brief Gives an image an index
         * @param view The image view, in SHADER_READ_ONLY_OPTIMAL when sampled
         * @param sampler The sampler it is read with
         * @returns The slot holding the index; the index is reused once the slot is destroyed
         */
        [[nodiscard]] TextureSlot Add(vk::ImageView view, vk::Sampler sampler);

        /**
         * @brief Brings the set the frame binds up to date
         * @param frameSlot The frame slot about to be recorded, whose previous frame the GPU has finished
         * @param deletionQueue Takes a replaced bindless set until the GPU is done with it
         * @param retireValues When the GPU is done with the frames recorded so far
         */
        void BeginFrame(std::uint32_t frameSlot, DeletionQueue& deletionQueue, const QueueTimelineValues& retireValues);

        /// @brief The set to bind at set 2 for `frameSlot`
        [[nodiscard]] vk::DescriptorSet Set(std::uint32_t frameSlot) const;
        /// @brief The layout of set 2
        [[nodiscard]] vk::DescriptorSetLayout Layout() const noexcept { return *m_layout; }

    private:
        friend class TextureSlot;

        /// @brief What an index currently holds; `version` changes whenever it is (re)assigned
        struct Entry
        {
            vk::ImageView view{};
            vk::Sampler sampler{};
            std::uint64_t version = 0;
        };

        /// @brief A set with its own pool, and what it holds
        struct FrameSet
        {
            vk::raii::DescriptorPool pool{ nullptr };
            vk::raii::DescriptorSet set{ nullptr };
            std::uint32_t capacity = 0;
            std::vector<std::uint64_t> writtenVersions; ///< per index: the entry version this set holds
        };

        void Free(std::uint32_t index);
        [[nodiscard]] FrameSet AllocateSet(std::uint32_t capacity) const;
        /// @brief Writes every index `frameSet` holds an old version of
        void Write(FrameSet& frameSet);

        const Context& m_context;
        std::uint32_t m_maxCapacity = 0;   ///< the layout's descriptor count: the device limit, at most 65536
        std::uint32_t m_capacity = 0;      ///< the count the sets are allocated with; doubles when full
        vk::raii::DescriptorSetLayout m_layout{ nullptr };
        std::vector<Entry> m_entries;      ///< index 0 is never handed out
        std::vector<std::uint32_t> m_free; ///< freed indices, reused first
        std::uint64_t m_nextVersion = 1;
        std::vector<FrameSet> m_sets;      ///< one with USE_BINDLESS, else one per frame slot
    };
}
