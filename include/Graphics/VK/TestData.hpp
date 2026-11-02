#pragma once

#include <cstdint>
#include <vector>
#include <Graphics/VK/VertexBuffer.hpp>

const std::vector<Droplet::Graphics::VK::Vertex> G_VERTICES = {
    {{-0.5f , -0.5f , 0.0f} , {0.0f , -1.0f , 0.0f }, {1.0f , 0.0f}},
    {{0.5f , -0.5f , 0.0f} , {0.0f , -1.0f , 0.0f} , {0.0f , 0.0f}},
    {{0.5f , 0.5f , 0.0f }, {0.0f , -1.0f , 0.0f }, {0.0f , 1.0f }},
    {{-0.5f , 0.5f , 0.0f} , {0.0f , -1.0f , 0.0f }, {1.0f , 1.0f }},

    {{-0.5f , -0.5f , -0.5f} , {0.0f , -1.0f , 0.0f} , {1.0f , 0.0f}},
    {{0.5f , -0.5f , -0.5f} , {0.0f , -1.0f , 0.0f} , {0.0f , 0.0f}},
    {{0.5f , 0.5f , -0.5f} , {0.0f , -1.0f , 0.0f} , {0.0f , 1.0f}},
    {{-0.5f , 0.5f , -0.5f} , {0.0f , -1.0f , 0.0f} , {1.0f , 1.0f}}
};

const std::vector<uint32_t> G_INDICES = {
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4 
};
