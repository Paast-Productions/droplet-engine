#pragma once

#include <array>
#include <cstdint>
#include <cstddef>
#include <functional>
#include <string>
#include <vector>
#include <optional>
#include <utility>

#include "Integration/Core/Timeline.hpp"

namespace RenderGraph
{
    // ============================================================
    // Common types
    // ============================================================

    /// Mirrors extent2D in vulkan while being platform independent
    struct Extent2D
    {
        std::uint32_t width{0};  ///< width of extent
        std::uint32_t height{0}; ///< height of extent
        /// Extend2D comparator operator
        friend bool operator==(Extent2D, Extent2D) = default;
    };

    // ============================================================
    // Queues
    // ============================================================
    
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

    /// Number of queues supported
    inline constexpr std::size_t QUEUE_COUNT = static_cast<std::size_t>(Queue::Count);
    
    /// index of the queue type in 
    constexpr std::size_t ToIndex(Queue queue) noexcept { return static_cast<std::size_t>(queue); }

    /// Bit set with bit ToIndex(queue) per queue
    using QueueMask = std::uint32_t;

    // ============================================================
    // Usage
    // ============================================================

    /// describes how a resource will be used
    enum class Usage : std::uint8_t
    {
        None,
        ColorWrite,   ///< image
        DepthWrite,   ///< image
        SampledRead,  ///< image
        StorageWrite, ///< image or buffer; a shader may read it too
        TransferSrc,  ///< image or buffer
        TransferDst,  ///< image or buffer
        Present,      ///< image; only as ImportOptions::requiredFinal
        StorageRead,  ///< image or buffer
        UniformRead,  ///< buffer
        IndirectRead, ///< buffer: draw or dispatch arguments
        VertexRead,   ///< buffer
        IndexRead     ///< buffer
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

    /// Bit set with UsageBit(usage) per usage
    using UsageMask = std::uint32_t;
    
    /// Get bit for usage in a usage mask, Compiled::UsageMasks
    constexpr UsageMask UsageBit(Usage usage) noexcept
    {
        return 1u << static_cast<uint32_t>(usage);
    }

    /// @brief Returns true if usage performs a write op
    constexpr bool IsWrite(Usage usage) noexcept
    {
        return usage == Usage::ColorWrite ||
            usage == Usage::DepthWrite ||
            usage == Usage::StorageWrite ||
            usage == Usage::TransferDst;
    }

    // ============================================================
    // Resources
    // ============================================================

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
    /// @param kind The kind of resource being checked
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

    /// A resource's unique id/index in the graph. Separate from GUID. 
    struct ResourceId
    {
        /// invalid ResourceId index
        static constexpr std::uint32_t INVALID = 0xFFFFFFFFu;
        std::uint32_t index{INVALID}; ///< acts as id and also index entry in steps

        /// returns true if index is not INVALID
        [[nodiscard]] constexpr bool IsValid() const noexcept { return index != INVALID; }
        /// bool comparator of two ResourceId
        friend constexpr bool operator==(ResourceId, ResourceId) = default;
    };

    /// state of a resource in the timeline
    struct ResourceState
    {
        Usage usage{Usage::None};       ///< ColorWrite, SampledRead, Present etc.
        Queue queue{Queue::Graphics};   ///< Which queue to submit to.
        TimelineValue value{0};         ///< What timeline semaphore value to wait for
    };

    struct ImportOptions
    {
        Usage requiredFinal{Usage::None}; ///< final state of the resource
        bool concurrent{false}; ///< image may be used on several queues (created w/ concurrent sharing flag)
        bool waitExternal{false}; ///< wait for a semaphore value
        bool signalExternal{false}; ///< signal to wait for a semaphore value
    };

    /// defines what width and height mean for an image texture
    enum class SizeClass
    {
        Absolute,           ///< width and height are pixels     
        SwapchainRelative,  ///< width and height are scale of swapchain extent
        InputRelative       ///< width and height are scale of TransientDesc::relativeTo extent
    };

    /// Describes a transient image resource in the graph
    struct TransientDesc
    {
        SizeClass sizeClass{SizeClass::SwapchainRelative}; ///< default, this describes a swapchain
        float width{1}; ///< pixels for absolute, else scale factor of swapchain extent
        float height{1}; ///< pixels for absolute, else scale factor of swapchain extent
        ResourceId relativeTo{}; ///< If extents are not pixels, id of the resource this scales
        std::uint32_t format{0};    ///< texture setting
        std::uint32_t mipLevels{1}; ///< texture setting
        std::uint32_t layers{1};    ///< texture setting
        std::uint32_t samples{1};   ///< texture setting
    };

    /// Description of a buffer the render graph creates that is not accessed outside the graph
    struct TransientBufferDesc
    {
        std::uint64_t size{0}; ///< bytes
    };

    /// @brief Non-owning info about a resource
    struct ResourceInfo
    {
        std::string name; ///< Debug info name
        std::uint64_t externalKey{0}; ///< GUID of the imported (external) resource. 0 means this is graph local 
        ResourceState initialState; ///< What the resource's state is going into the frame
        ImportOptions options;
        ResourceKind kind{ResourceKind::Image}; ///< Buffer or Image
        std::optional<TransientDesc> transient{}; ///< set if the graph creates an image for the resource
        std::optional<TransientBufferDesc> transientBuffer{}; ///< set if the graph creates a buffer for the resource

        /// Returns true if created by graph rather than imported
        [[nodiscard]] bool IsTransient() const noexcept { return transient.has_value() || transientBuffer.has_value(); }
        
        /// Returns true if resource is a buffer
        [[nodiscard]] bool IsBuffer() const noexcept { return kind == ResourceKind::Buffer; }
    };

    // ============================================================
    // Passes
    // ============================================================

    /// Id and Index for a pass in Compiled::passes
    using PassId = std::uint32_t;

    /// What type of access a resource is doing
    struct Access
    {
        ResourceId resource;        ///< id + index of the resource
        Usage usage{Usage::None};   ///< what the access is doing (write color, read sampler, present, etc.)
        bool history{false};        ///< whether to read state from previous frame
    };

    /// information about a single graph pass
    struct PassInfo
    {
        std::string name; ///< debug name
        std::vector<Access> accesses; ///<
        Queue queue = Queue::Graphics; ///< queue output gets submitted to
        bool sideEffect = false; ///< Never gets culled by the graph
        std::vector<std::string> setupErrors; /// stored errors
    };

    // ============================================================
    // Graph
    // ============================================================

    /// data held by a Graph
    struct GraphData
    {
        std::vector<ResourceInfo> resources; ///< info about every resource in the graph
        std::vector<PassInfo> passes; ///< info about every pass in the graph
    };

    // ============================================================
    // Compiled output
    // ============================================================

    /// Position in Compiled::steps
    using StepIndex = std::uint32_t;
    /// Position in Compiled::submissions
    using SubmissionIndex = std::uint32_t;
    
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

    /// tracks the last usage of a specific resource
    struct LastUse
    {
        bool touched{false};
        Queue queue{Queue::Graphics};
        SubmissionIndex submission{0}; ///< index into compiled::submissions, ResourceId
    };
    
    /// tracks the steps that will use the resource, its first usage and its last usage
    struct Lifetime
    {
        /// An UNUSED state never fired
        static constexpr std::uint32_t UNUSED = 0xFFFFFFFFu;
        StepIndex firstStep{UNUSED}; ///< first step index
        StepIndex lastStep{UNUSED};  ///< last step index

        /// returns true if first step it not unused
        [[nodiscard]] constexpr bool IsUsed() const noexcept { return firstStep != UNUSED; }

        /// returns true if this lifetime overlaps with another lifetime in the graph
        [[nodiscard]] constexpr bool Overlaps(const Lifetime& other) const
        {
            return IsUsed() && other.IsUsed() && firstStep <= other.lastStep && other.firstStep <= lastStep;
        }
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

    // ============================================================
    // Functions
    // ============================================================
    
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
