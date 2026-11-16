/**
* @file RenderGraphTypes.hpp
* @brief Defines types for the RenderGraph that are Vulkan-independent
*/
#pragma once

#include <array>
#include <cstdint>
#include <cstddef>
#include <functional>
#include <string>
#include <vector>
#include <optional>

#include "Core/Timeline.hpp"

namespace Droplet::Graphics::RenderGraph
{
    /// Bit set with bit ToIndex(queue) per queue
    using QueueMask = std::uint32_t;
    /// Bit set with UsageBit(usage) per usage
    using UsageMask = std::uint32_t;
    /// Position in Compiled::steps
    using StepIndex = std::uint32_t;
    /// Position in Compiled::submissions
    using SubmissionIndex = std::uint32_t;
    /// Index in the 
    using PassId = std::uint32_t;
    
    
    /// describes the queue family that a command would be submitted to
    enum class Queue
    {
        Graphics,
        Compute,
        Transfer,
        Count
    };

    /// Helper function to get Queue enum value as a string
    constexpr const char* ToString(Queue queue)
    {
        switch (queue)
        {
        case Queue::Graphics: return "Graphics";
        case Queue::Compute: return "Compute";
        case Queue::Transfer: return "Transfer";
        case Queue::Count: return "Count";
        }
        std::unreachable();
    }

    inline constexpr std::size_t QUEUE_COUNT = static_cast<std::size_t>(Queue::Count);
    constexpr std::size_t ToIndex(Queue queue) noexcept { return static_cast<std::size_t>(queue); }

    /// describes how the resource will be used
    enum class Usage : std::uint8_t
    {
        None,
        ColorWrite, ///< image
        DepthWrite, ///< image
        SampledRead, ///< image
        StorageWrite, ///< image or buffer; a shader may read it too
        TransferSrc, ///< image or buffer
        TransferDst, ///< image or buffer
        Present, ///< image; only as ImportOptions::requiredFinal
        StorageRead, ///< image or buffer
        UniformRead, ///< buffer
        IndirectRead, ///< buffer: draw or dispatch arguments
        VertexRead, ///< buffer
        IndexRead ///< buffer
    };

    /// Helper function to get Usage enum value as a string
    constexpr const char* ToString(Usage usage)
    {
        switch (usage)
        {
        case Usage::None: return "None";
        case Usage::ColorWrite: return "ColorWrite";
        case Usage::DepthWrite: return "DepthWrite";
        case Usage::SampledRead: return "SampledRead";
        case Usage::StorageWrite: return "StorageWrite";
        case Usage::TransferSrc: return "TransferSrc";
        case Usage::TransferDst: return "TransferDst";
        case Usage::Present: return "Present";
        case Usage::StorageRead: return "StorageRead";
        case Usage::UniformRead: return "UniformRead";
        case Usage::IndirectRead: return "IndirectRead";
        case Usage::VertexRead: return "VertexRead";
        case Usage::IndexRead: return "IndexRead";
        default: return "unknown";
        }
    }

    /** 
     * @brief Get bit for usage in a usage mask, Compiled::UsageMasks
     */
    constexpr UsageMask UsageBit(Usage usage) noexcept
    {
        return 1u << static_cast<uint32_t>(usage);
    }

    constexpr bool IsWrite(Usage usage) noexcept
    {
        return usage == Usage::ColorWrite ||
            usage == Usage::DepthWrite ||
            usage == Usage::StorageWrite ||
            usage == Usage::TransferDst;
    }

    /// @brief A resource is either an Image or a Buffer
    enum class ResourceKind : std::uint8_t
    {
        Image,
        Buffer
    };

    /// Helper function to get ResourceKind enum value as a string
    constexpr const char* ToString(ResourceKind kind)
    {
        return kind == ResourceKind::Image ? "image" : "buffer";
    }

    /// @brief See if usage is valid for a resource (image or buffer)
    /// @param kind The kind of resource being chekced
    /// @param usage The type of usage being tested
    /// returns true if valid
    constexpr bool UsageFits(ResourceKind kind, Usage usage) noexcept
    {
        switch (usage)
        {
        case Usage::ColorWrite:
        case Usage::DepthWrite:
        case Usage::SampledRead:
        case Usage::Present:
            return kind == ResourceKind::Image;
        case Usage::UniformRead:
        case Usage::IndirectRead:
        case Usage::VertexRead:
        case Usage::IndexRead:
            return kind == ResourceKind::Buffer;
        default:
            return true;
        }
    }

    /// defines what width and height for image texture mean
    enum class SizeClass
    {
        Absolute, ///< width and height of image are pixels     
        SwapchainRelative, ///< width and height scale swapchain extent
        InputRelative ///< width and height scale extent of TransientDesc::relativeTo
    };

    struct ResourceId
    {
        static constexpr std::uint32_t INVALID = 0xFFFFFFFFu;
        std::uint32_t index{INVALID}; ///< acts as id and also index entry in

        /// returns true if index is not INVALID
        [[nodiscard]] constexpr bool IsValid() const noexcept { return index != INVALID; }
        friend constexpr bool operator==(ResourceId, ResourceId) = default;
    };

    /// state of a resource in the timeline
    struct ResourceState
    {
        Usage usage{Usage::None};
        Queue queue{Queue::Graphics};
        TimelineValue value{0};
    };

    struct ImportOptions
    {
        Usage requiredFinal{Usage::None}; ///< final state of the resource
        bool concurrent{false}; ///< image may be used on several queues (created w/ concurrent sharing flag)
        bool waitExternal{false}; ///< wait for a semaphore value
        bool signalExternal{false}; ///< signal to wait for a semaphore value
    };

    /// mirrors extent2D in vulkan while being platform independent
    struct Extent2D
    {
        std::uint32_t width{0};
        std::uint32_t height{0};
        friend bool operator==(Extent2D, Extent2D) = default;
    };

    struct TransientDesc
    {
        SizeClass sizeClass{SizeClass::SwapchainRelative}; ///< default, this describes a swapchain
        float width{1}; ///< pixels for absolute, else scale factor of swapchain extent
        float height{1}; ///< pixels for absolute, else scale factor of swapchain extent
        ResourceId relativeTo{};
        std::uint32_t format{0};
        std::uint32_t mipLevels{1};
        std::uint32_t layers{1};
        std::uint32_t samples{1};
    };

    /// Description of a buffer the render graph creates that is not accessed outside the graph
    struct TransientBufferDesc
    {
        std::uint64_t size{0}; ///< bytes
    };

    /// tracks the steps that will use the resource, its first usage and its last usage
    struct Lifetime
    {
        static constexpr std::uint32_t UNUSED = 0xFFFFFFFFu;
        StepIndex firstStep{UNUSED};
        StepIndex lastStep{UNUSED};

        [[nodiscard]] constexpr bool IsUsed() const noexcept { return firstStep != UNUSED; }

        [[nodiscard]] constexpr bool Overlaps(const Lifetime& other) const
        {
            return IsUsed() && other.IsUsed() && firstStep <= other.lastStep && other.firstStep <= lastStep;
        }
    };

    struct Access
    {
        ResourceId resource;
        Usage usage{Usage::None};
        bool history{false}; ///< whether to read previous state
    };

    /// @brief non-owning info about some resource
    struct ResourceInfo
    {
        std::string name; ///< Debug info
        std::uint64_t externalKey{0}; ///< key 
        ResourceState initialState;
        ImportOptions options;
        ResourceKind kind{ResourceKind::Image};
        std::optional<TransientDesc> transient{}; ///< set if the graph creates an image for the resource
        std::optional<TransientBufferDesc> transientBuffer{}; ///< set if the graph creates a buffer for the resource

        /// Returns true if created by graph rather than imported
        [[nodiscard]] bool IsTransient() const noexcept { return transient.has_value() || transientBuffer.has_value(); }

        [[nodiscard]] bool IsBuffer() const noexcept { return kind == ResourceKind::Buffer; }
    };

    /// information about a single graph pass
    struct PassInfo
    {
        std::string name; ///< debug name
        std::vector<Access> accesses;
        Queue queue = Queue::Graphics;
        bool sideEffect = false;
        std::vector<std::string> setupErrors; /// stored errors
    };

    /// data held by a Graph
    struct GraphData
    {
        std::vector<ResourceInfo> resources;
        std::vector<PassInfo> passes;
    };
    
    /// describes a barrier that needs to be set for resource with resourceId
    struct Barrier
    {
        ResourceId resourceId;
        Usage from{Usage::None};
        Usage to{Usage::None};
        Queue fromQueue{Queue::Graphics}; ///< queue of the previous access
    };

    /// A single step in a graph pass
    struct Step
    {
        PassId pass;
        Queue queue{Queue::Graphics};
        std::uint32_t level{0}; ///< level in the graph's dependency tree. 0 needs no other passes
        std::vector<Barrier> barriers;
    };

    /// wait on earlier work in the timeline
    struct CarryWait
    {
        Queue queue{Queue::Graphics};
        TimelineValue value{0};
    };

    /// a run of consecutive steps on one queue, one command buffer, one submit
    struct Submission
    {
        Queue queue{Queue::Graphics};
        StepIndex firstStep{0};
        std::uint32_t stepCount{0};
        std::uint32_t signalIndex{0};
        std::vector<std::uint32_t> waits;
        std::vector<CarryWait> carryWaits;
        std::vector<ResourceId> waitExternal;
        std::vector<ResourceId> signalExternal;
        std::vector<Barrier> epilogue;
    };

    /// tracks the last usage of a 
    struct LastUse
    {
        bool touched{false};
        Queue queue{Queue::Graphics};
        SubmissionIndex submission{0}; ///< index into compiled::submissions
    };
    
    /// A compiled render graph, ready to be executed
    struct Compiled
    {
        bool ok = false;
        std::vector<Step> steps;
        std::vector<Submission> submissions;
        std::array<std::uint32_t, QUEUE_COUNT> signalsPerQueue{};
        std::vector<Usage> finalUsage;
        std::vector<LastUse> lastUse;
        std::vector<Lifetime> lifetimes; ///< per resource, the steps it is used in, used for transient allocation
        std::vector<UsageMask> usageMasks; ///< per resource, UsageBit of every usage by a live pass or epilogue
        std::vector<QueueMask> queueMasks; ///< per resource, bit ToIndex(queue) of every queue a live pass uses it on
        std::vector<PassId> culled;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
    };
    
    /// compile graph data into something that can be baked into executable commnads
    Compiled Compile(const GraphData &graph);

    /**
     * Resolve a resource's size in pixels relative to swapchain
     * @param graph graph holding the resource
     * @param id ResourceId of the resource
     * @param swapChainExtent the extent that manages
     * @param importedEvent returns the extent of an imported resource
     * @return size of the extent2d, at least 1x1 for a transient
     * @throws std::logic_error if InputRelative references are cyclical
     */
    Extent2D ResolveExtent(
        const GraphData &graph,
        ResourceId id,
        Extent2D swapChainExtent,
        const std::function<Extent2D(ResourceId)> &importedEvent);
    
    /// dump contents of entire graph as a string
    std::string Dump(const GraphData &graph, const Compiled &compiled);
}
