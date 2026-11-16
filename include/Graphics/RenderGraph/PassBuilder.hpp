#pragma once

#include "Graphics/RenderGraph/RenderGraphTypes.hpp"

namespace Droplet::Graphics::RenderGraph
{
    /// builds a render graph that is consumed by rendering
    class PassBuilder
    {
    public:
        explicit PassBuilder(PassInfo&pass) : m_pass(pass){}
        
        /// Add read without history to the pass
        void Read(ResourceId resourceId, Usage usage = Usage::SampledRead) const
        {   
            Add(resourceId, usage, false, false);
        }
        
        /// Add history read to the pass
        void ReadPrevious(ResourceId resourceId, Usage usage = Usage::SampledRead) const
        {
            Add(resourceId, usage, false, true);
        }
        
        /// add write to the pass
        void Write(ResourceId resourceId, Usage usage = Usage::ColorWrite) const
        {
            Add(resourceId, usage, true, false);
        }
        
        /// Never culled (presents, readbacks, anything observable outside the graph)
        void SideEffect() const { m_pass.sideEffect = true; }
        
        /// Which queue records this pass (default Graphics)
        void OnQueue(Queue queue) const { m_pass.queue = queue; }

    private:
        /// add a resource access to the pass
        void Add(ResourceId resourceId, Usage usage, bool write, bool history) const
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