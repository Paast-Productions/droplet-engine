/**
* @file fullscreen.vs.slang
* @brief VertexShader that outputs a fullscreen Quad
* @author Sebastian Lorensson
*/
#language slang 2026

import "Modules/shared_types.md.slang"; // vertex and post-process

[shader("vertex")]
Vertex::OUT_2PUV vMain(uint vertexID: SV_VertexID)
{
    // creates a triangle that takes up the entire screen
    // url: https://www.saschawillems.de/blog/2016/08/13/vulkan-tutorial-on-rendering-a-fullscreen-quad-without-buffers/
    Vertex::OUT_2PUV output;
    output.uv = float2((vertexID << 1) & 2, vertexID & 2);
    output.pos = float4(output.uv * 2.0f + -1.0f, 0.0f, 1.0f);
    return output;
}