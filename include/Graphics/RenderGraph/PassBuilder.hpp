#pragma once

#include "Graphics/RenderGraph/RenderGraphTypes.hpp"

namespace RenderGraph
{
    /**
     * @brief Orders PassInfo by resource access.
     */
    class PassBuilder
    {
    public:
        /// This holds a reference to the PassInfo to ensure it lives to the end of the frame.
        explicit PassBuilder(PassInfo& pass) : m_pass(pass)
        {
        }


        void Read(
            ResourceId resourceId,
            Usage usage = Usage::SampledRead)
        {
            Add(resourceId, usage, false, false);
        }

        void ReadPrevious(ResourceId resourceId, Usage usage = Usage::SampledRead)
        {
            Add(resourceId, usage, false, true);
        }
        
        void Write(ResourceId resourceId, Usage usage = Usage::ColorWrite)
        {
            Add(resourceId, usage, true, false);
        }
        
        /// Never culled (presents, readbacks, anything observable outside the graph).
        void SideEffect() const { m_pass.sideEffect = true; }
        
        /// Which queue records this pass (default Graphics).
        void OnQueue(Queue queue) const { m_pass.queue = queue; }

    private:
        /**
         * Adds a resource to the PassInfo as an access
         * @param resourceId id of the resource
         * @param usage @copybrief RenderGraph::Usage
         * @param write true if the resource is expecting to write
         * @param history true if resource needs access to history (state last frame)
         */
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

        PassInfo& m_pass; ///< held until the end of the frame submission.
    };
}
