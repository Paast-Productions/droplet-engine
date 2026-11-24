#pragma once

#include <cstddef>
#include <functional>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

#include "Graphics/RenderGraph/RenderGraphTypes.hpp"
#include "Graphics/RenderGraph/PassBuilder.hpp"

namespace RenderGraph
{
    /// @brief main Render Graph class
    /// @tparam Ctx is whatever the renderer backend hands to pass callbacks, like a struct holding a command buffer
    template <class Ctx>
    class Graph
    {
    public:
        using ExecuteFn = std::function<void(Ctx&)>; ///< execute functions will vary, so they are passed as lambdas

        /// 
        ResourceId Import(
            std::string name,
            std::uint64_t externalKey,
            ResourceState current,
            ImportOptions options = {})
        {
            m_data.resources.push_back({
                .name = std::move(name),
                .externalKey = externalKey,
                .initialState = current,
                .options = options
            });

            return {static_cast<std::uint32_t>(m_data.resources.size() - 1)};
        }

        /// Shorthand for a resource left in current on the graphics queue with no pending timeline value.
        ResourceId Import(
            std::string name,
            std::uint64_t externalKey,
            Usage current, ImportOptions
            options = {})
        {
            return Import(std::move(name), externalKey, ResourceState{
                              .usage = current,
                              .queue = Queue::Graphics,
                              .value = 0
                          },
                          options);
        }

        /// @brief A buffer that lives outside the graph (state carried between frames like an imported image).
        ResourceId ImportBuffer(
            std::string name,
            std::uint64_t externalKey,
            ResourceState current,
            ImportOptions options = {})
        {
            const ResourceId id = Import(std::move(name), externalKey, current, options);
            m_data.resources.back().kind = ResourceKind::Buffer;
            return id;
        }

        /// @brief A buffer the graph creates for this frame; its contents do not survive the frame.
        ResourceId CreateBuffer(std::string name, const TransientBufferDesc& desc)
        {
            ResourceInfo info;
            info.name = std::move(name);
            info.kind = ResourceKind::Buffer;
            info.transientBuffer = desc;
            m_data.resources.push_back(std::move(info));
            return {static_cast<std::uint32_t>(m_data.resources.size() - 1)};
        }

        ResourceId Create(std::string name, const TransientDesc& desc)
        {
            ResourceInfo info;
            info.name = std::move(name);
            info.transient = desc;
            m_data.resources.push_back(std::move(info));
            return {static_cast<std::uint32_t>(m_data.resources.size() - 1)};
        }

        /// setup is called immediately with a PassBuilder; execute is called during Execute()
        template <class Setup>
        PassId AddPass(std::string name, Setup&& setup, ExecuteFn execute = {})
        {
            PassInfo info;
            info.name = std::move(name);
            PassBuilder builder{info};
            setup(builder);
            m_data.passes.push_back(std::move(info));
            m_exec.push_back(std::move(execute));
            return static_cast<PassId>(m_data.passes.size() - 1);
        }

        Compiled Compile() const { return RenderGraph::Compile(m_data); }
        const GraphData& Data() const { return m_data; }

        /// @brief Executes stored data
        /// @details Recorder hooks, called per submission in submit order:
        ///   Ctx& Begin(const Submission&);                    open a command buffer for it
        ///   void Barriers(Ctx&, std::span<const Barrier>);     record transitions (also used for the epilogue)
        ///   void End(Ctx&, const Submission&);                 close it and submit with the waits/signals it describes
        template <class Recorder>
        void Execute(const Compiled& compiled, Recorder& recorder) const
        {
            if (!compiled.ok)
                throw std::logic_error("RenderGraph::Execute called with a graph that failed to compile");

            // iterate over all submissions
            for (const Submission& submission : compiled.submissions)
            {
                Ctx& context = recorder.Begin(submission);
                for (std::uint32_t stepOffset = 0; stepOffset < submission.stepCount; ++stepOffset)
                {
                    const Step& step = compiled.steps[submission.firstStep + stepOffset];
                    if (!step.barriers.empty())
                        recorder.Barriers(context, std::span<const Barrier>(step.barriers));
                    if (m_exec[step.pass])
                        m_exec[step.pass](context);
                }
                if (!submission.epilogue.empty())
                    recorder.Barriers(context, std::span<const Barrier>(submission.epilogue));
                recorder.End(context, submission);
            }
        }

    private:
        GraphData m_data;
        std::vector<ExecuteFn> m_exec;
    };
}
