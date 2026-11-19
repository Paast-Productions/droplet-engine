#pragma once

#include "Integration/RenderGraph/RenderGraphTypes.hpp"

namespace RenderGraph
{
    /// Builds a RenderPass to be consumed by the rendering
    class PassBuilder
    {
    public:
        explicit PassBuilder(PassInfo& pass) : m_pass(pass)
        {
        }

        void Read(
            ResourceId resourceId,
            Usage usage = Usage::SampledRead)
        {
            Add(resourceId, usage, false, false);
        }

        void ReadPrevious(ResourceId resourceId, Usage usage = Usage::SampledRead) { Add(resourceId, usage, false, true); }
        void Write(ResourceId resourceId, Usage usage = Usage::ColorWrite) { Add(resourceId, usage, true, false); }
        
        /// Never culled (presents, readbacks, anything observable outside the graph).
        void SideEffect() const { m_pass.sideEffect = true; }
        
        /// Which queue records this pass (default Graphics).
        void OnQueue(Queue queue) const { m_pass.queue = queue; }

    private:
        void Add(ResourceId resourceId, Usage usage, bool write, bool history)
        {
            if (usage == Usage::None)
                m_pass.setupErrors.emplace_back("access with Usage::None");
            else if (usage == Usage::Present)
                m_pass.setupErrors.emplace_back("Usage::Present is only valid as ImportOptions::requiredFinal");
            else if (IsWrite(usage) != write)
                m_pass.setupErrors.push_back(
                    std::string(write ? "Write" : "Read") + " declared with " + ToString(usage));
            
            m_pass.accesses.push_back(
                {
                    .resource = resourceId,
                    .usage = usage,
                    .history = history
                });
        }

        PassInfo& m_pass;
    };
}
