/**
 * @file TextureTable.cpp
 * @brief Vulkan::TextureTable implementation
 */
#include "Graphics/Vulkan/Resources/TextureTable.hpp"

#include <algorithm>
#include <stdexcept>

#include "Graphics/Vulkan/Abstraction/Device.hpp"
#include "Graphics/Vulkan/Core/Context.hpp"
#include "Graphics/Vulkan/Resources/DeletionQueue.hpp"

namespace Droplet::Graphics::Vulkan
{
    namespace
    {
        constexpr std::uint32_t FIRST_CAPACITY = 64;
        constexpr std::uint32_t LIMIT_CAPACITY = 65536;
    }

    TextureSlot::~TextureSlot()
    {
        if (m_table)
            m_table->Free(m_index);
    }

    TextureSlot& TextureSlot::operator=(TextureSlot&& other) noexcept
    {
        if (this != &other)
        {
            if (m_table)
                m_table->Free(m_index);
            m_table = std::exchange(other.m_table, nullptr);
            m_index = other.m_index;
        }
        return *this;
    }

    TextureTable::TextureTable(const Context& context, std::uint32_t framesInFlight)
        : m_context(context)
    {
        const vk::PhysicalDeviceLimits& limits = context.GetPhysicalDevice().getProperties().limits;
        m_maxCapacity = std::min({ LIMIT_CAPACITY, limits.maxPerStageDescriptorSamplers, limits.maxPerStageDescriptorSampledImages,
                                   limits.maxDescriptorSetSamplers, limits.maxDescriptorSetSampledImages });
        m_capacity = std::min(FIRST_CAPACITY, m_maxCapacity);

        // one binding, an array of up to m_maxCapacity; sets are allocated with the current capacity
        const vk::DescriptorBindingFlags bindingFlags = USE_BINDLESS
            ? vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eVariableDescriptorCount
              | vk::DescriptorBindingFlagBits::eUpdateAfterBind | vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending
            : vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eVariableDescriptorCount;
        const vk::DescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{ .bindingCount = 1, .pBindingFlags = &bindingFlags };
        const vk::DescriptorSetLayoutBinding binding{
            .binding = 0,
            .descriptorType = vk::DescriptorType::eCombinedImageSampler,
            .descriptorCount = m_maxCapacity,
            .stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment };
        m_layout = vk::raii::DescriptorSetLayout(context.GetDevice(), vk::DescriptorSetLayoutCreateInfo{
            .pNext = &flagsInfo,
            .flags = USE_BINDLESS ? vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool
                                  : vk::DescriptorSetLayoutCreateFlags{},
            .bindingCount = 1,
            .pBindings = &binding });

        m_entries.resize(1); // index 0 is never handed out
        const std::uint32_t setCount = USE_BINDLESS ? 1 : framesInFlight;
        for (std::uint32_t setIndex = 0; setIndex < setCount; ++setIndex)
            m_sets.push_back(AllocateSet(m_capacity));
    }

    TextureTable::~TextureTable() = default;

    TextureSlot TextureTable::Add(vk::ImageView view, vk::Sampler sampler)
    {
        std::uint32_t index = 0;
        if (!m_free.empty())
        {
            index = m_free.back();
            m_free.pop_back();
        }
        else
        {
            index = static_cast<std::uint32_t>(m_entries.size());
            if (index >= m_maxCapacity)
                throw std::runtime_error("TextureTable: more textures than the device allows in one array");
            m_entries.emplace_back();
            while (index >= m_capacity)
                m_capacity = std::min(m_capacity * 2, m_maxCapacity); // the sets grow in BeginFrame
        }
        m_entries[index] = Entry{ .view = view, .sampler = sampler, .version = m_nextVersion++ };
        return TextureSlot(*this, index);
    }

    void TextureTable::Free(std::uint32_t index)
    {
        // the GPU is done with the image: the index can hold another one
        m_entries[index].view = vk::ImageView{};
        m_entries[index].sampler = vk::Sampler{};
        m_free.push_back(index);
    }

    void TextureTable::BeginFrame(std::uint32_t frameSlot, DeletionQueue& deletionQueue, const QueueTimelineValues& retireValues)
    {
        FrameSet& frameSet = m_sets[USE_BINDLESS ? 0 : frameSlot];
        if (frameSet.capacity < m_capacity)
        {
            // a larger set: earlier frames may still use the old bindless set, a frame slot's own set is idle
            FrameSet larger = AllocateSet(m_capacity);
            if (USE_BINDLESS)
                deletionQueue.Retire(std::move(frameSet), retireValues);
            frameSet = std::move(larger);
        }
        Write(frameSet);
    }

    vk::DescriptorSet TextureTable::Set(std::uint32_t frameSlot) const
    {
        return *m_sets[USE_BINDLESS ? 0 : frameSlot].set;
    }

    TextureTable::FrameSet TextureTable::AllocateSet(std::uint32_t capacity) const
    {
        FrameSet frameSet;
        const vk::DescriptorPoolSize poolSize{ .type = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = capacity };
        frameSet.pool = vk::raii::DescriptorPool(m_context.GetDevice(), vk::DescriptorPoolCreateInfo{
            .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet
                     | (USE_BINDLESS ? vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind : vk::DescriptorPoolCreateFlags{}),
            .maxSets = 1,
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize });

        const vk::DescriptorSetVariableDescriptorCountAllocateInfo countInfo{ .descriptorSetCount = 1, .pDescriptorCounts = &capacity };
        const vk::DescriptorSetLayout layout = *m_layout;
        vk::raii::DescriptorSets sets(m_context.GetDevice(), vk::DescriptorSetAllocateInfo{
            .pNext = &countInfo,
            .descriptorPool = *frameSet.pool,
            .descriptorSetCount = 1,
            .pSetLayouts = &layout });
        frameSet.set = std::move(sets.front());
        frameSet.capacity = capacity;
        frameSet.writtenVersions.assign(capacity, 0);
        return frameSet;
    }

    void TextureTable::Write(FrameSet& frameSet)
    {
        std::vector<vk::DescriptorImageInfo> images;
        std::vector<std::uint32_t> indices;
        const auto count = std::min<std::size_t>(m_entries.size(), frameSet.capacity);
        for (std::uint32_t index = 1; index < count; ++index)
        {
            const Entry& entry = m_entries[index];
            if (!entry.view || frameSet.writtenVersions[index] == entry.version)
                continue;
            images.push_back({ .sampler = entry.sampler, .imageView = entry.view, .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal });
            indices.push_back(index);
            frameSet.writtenVersions[index] = entry.version;
        }

        std::vector<vk::WriteDescriptorSet> writes;
        writes.reserve(indices.size());
        for (std::size_t writeIndex = 0; writeIndex < indices.size(); ++writeIndex)
        {
            writes.push_back({
                .dstSet = *frameSet.set,
                .dstBinding = 0,
                .dstArrayElement = indices[writeIndex],
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .pImageInfo = &images[writeIndex] });
        }
        if (!writes.empty())
            m_context.GetDevice().updateDescriptorSets(writes, {});
    }
}
