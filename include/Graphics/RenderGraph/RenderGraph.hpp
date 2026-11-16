#pragma once

#include <cstddef>
#include <functional>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

#include "RenderGraphTypes.hpp"

namespace Droplet::Graphics::RenderGraph
{
    template <class Ctx>
    class Graph
    {
    public:
        using ExecuteFn = std::function<void(Ctx&)>;

        ResourceId Import(
            std::string name,
            std::uint64_t externalKey,
            ResourceState current,
            ImportOptions options)
        {
            m_data.resources.push_back({
                .name = std::move(name),
                .externalKey = externalKey,
                .initialState = current,
                .options = options
            });

            return {static_cast<std::uint32_t>(m_data.resources.size() - 1)};
        }

        
        
    private:
        GraphData m_data; ///< data held by this graph
        std::vector<ExecuteFn> m_exec; ///< compiled execution functions for each pass
    };
}
