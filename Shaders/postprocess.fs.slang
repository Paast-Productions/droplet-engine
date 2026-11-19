/*
*/
#language slang 2026

import "Modules/shared_types.md.slang"; // vertex and post-process
import "Modules/post_process.md.slang"; // vertex and post-process

[[vk::binding(0, 0)]]
Sampler2D sourceImage;

[[vk::push_constant]]
PostProcess::Parameters postProcess;

[shader("fragment")]
float4 fMain(Vertex::OUT_2PUV input) : SV_Target0
{
    const float4 color = sourceImage.Sample(input.uv);

    return PostProcess::apply(color, postProcess);
}
