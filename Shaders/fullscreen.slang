#language slang 2026

import "Modules/shared_types.slang"; // vertex and post-process

[[vk::binding(0, 0)]]
Sampler2D sourceImage;

[[vk::push_constant]]
PostProcess::Parameters postProcess;

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

// remove chrominance and leave only the 'brightness' of the color
// url: https://www.johndcook.com/blog/2009/08/24/algorithms-convert-color-grayscale/
float grayscale(float3 color)
{
    const float3 luminance = float3(0.2126, 0.7152, 0.0722);
    return dot(color, luminance);
}

// push contrasts from the 'center' by a factor of amount
float3 contrast(float3 color, float amount)
{
    return saturate((color.rgb - 0.5) * amount + 0.5);
}

// invert colours
float3 invert(float3 color) { return float3(1.0f - color);}

[shader("fragment")]
float4 fMain(Vertex::OUT_2PUV input) : SV_Target0
{
    const float4 color = sourceImage.Sample(input.uv);

    switch (postProcess.effect)
    {
    case PostProcess::Effect::Grayscale:
    {
        const float luminance = grayscale(color.rgb);

        return float4(luminance.xxx, color.a);
    }

    case PostProcess::Effect::Contrast:
    {
        const float3 contrasted = contrast(color.rgb, postProcess.amount);

        return float4(contrasted, color.a);
    }

    // invert all colours
    case PostProcess::Effect::Invert:
    {
        return float4(invert(color.rgb), color.a);
    }

    case PostProcess::Effect::Passthrough: // Passthrough
    default:
        return color;
    }
}
