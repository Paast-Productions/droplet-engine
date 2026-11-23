/**
* @file postprocess.cs.slang
* @brief Compute shaders of the post-process effects: cMain samples the source image and writes the destination image;
*        cMeasureLuminance and cApplyExposure are the two steps of auto exposure
*/
#language slang 2026

import "Modules/post_process.md.slang"; // effects and their parameters

[[vk::binding(0, 0)]]
Sampler2D sourceImage;

// rgba16f: sRGB formats cannot be storage images. Matches Vulkan::EffectPass::IMAGE_FORMAT
[[vk::binding(1, 0)]]
[[vk::image_format("rgba16f")]]
RWTexture2D<float4> destinationImage;

// carries a result from one step of an effect to a later one; element 0: the average luminance
[[vk::binding(2, 0)]]
RWStructuredBuffer<float> stepBuffer;

[[vk::push_constant]]
PostProcess::Parameters postProcess;

// Gaussian blur along one axis; a horizontal pass followed by a vertical one blurs in both
// texelStep: one pixel along the axis, in uv
// radius: reach in pixels (at most 32); sigma is half of it
float4 blurAlongAxis(float2 uv, float2 texelStep, float radius)
{
    const int reach = clamp(int(ceil(radius)), 1, 32);
    const float sigma = max(radius * 0.5f, 0.5f);

    float4 sum = float4(0.0f);
    float weightSum = 0.0f;
    for (int offset = -reach; offset <= reach; ++offset)
    {
        const float weight = exp(-0.5f * float(offset * offset) / (sigma * sigma));
        sum += weight * sourceImage.SampleLevel(uv + texelStep * float(offset), 0.0f);
        weightSum += weight;
    }
    return sum / weightSum;
}

// 8 x 8 pixels per work group; matches Vulkan::EffectPass::WORK_GROUP_SIZE
[shader("compute")]
[numthreads(8, 8, 1)]
void cMain(uint3 pixel : SV_DispatchThreadID)
{
    uint width;
    uint height;
    destinationImage.GetDimensions(width, height);

    // the last work groups reach past the image's edge
    if (pixel.x >= width || pixel.y >= height)
        return;

    // the centre of the pixel, so a source of the same size is read texel for texel
    const float2 size = float2(width, height);
    const float2 uv = (float2(pixel.xy) + 0.5f) / size;

    switch (postProcess.effect)
    {
    case PostProcess::Effect::BlurHorizontal:
        destinationImage[pixel.xy] = blurAlongAxis(uv, float2(1.0f / size.x, 0.0f), postProcess.amount);
        break;

    case PostProcess::Effect::BlurVertical:
        destinationImage[pixel.xy] = blurAlongAxis(uv, float2(0.0f, 1.0f / size.y), postProcess.amount);
        break;

    // per-pixel effects
    default:
        destinationImage[pixel.xy] = PostProcess::apply(sourceImage.SampleLevel(uv, 0.0f), postProcess);
        break;
    }
}

// samples per side of the grid auto exposure measures; one work group of 16 x 16 threads takes 16 samples each
static const uint MEASURE_GRID = 64;
static const uint MEASURE_THREADS = 256;
groupshared float logLuminanceSums[MEASURE_THREADS];

// auto exposure, step 1: the average luminance of the source image (geometric mean) into stepBuffer[0].
// One work group: Vulkan::EffectPass dispatches it once whatever the image size
[shader("compute")]
[numthreads(16, 16, 1)]
void cMeasureLuminance(uint threadIndex : SV_GroupIndex)
{
    const uint samplesPerThread = MEASURE_GRID * MEASURE_GRID / MEASURE_THREADS;
    float sum = 0.0f;
    for (uint sampleIndex = 0; sampleIndex < samplesPerThread; ++sampleIndex)
    {
        const uint gridIndex = threadIndex * samplesPerThread + sampleIndex;
        const float2 uv = (float2(gridIndex % MEASURE_GRID, gridIndex / MEASURE_GRID) + 0.5f) / float(MEASURE_GRID);
        const float luminance = PostProcess::grayscale(sourceImage.SampleLevel(uv, 0.0f).rgb);
        sum += log(max(luminance, 0.0001f));
    }
    logLuminanceSums[threadIndex] = sum;
    GroupMemoryBarrierWithGroupSync();

    // halve the number of partial sums until one is left
    for (uint stride = MEASURE_THREADS / 2; stride > 0; stride /= 2)
    {
        if (threadIndex < stride)
            logLuminanceSums[threadIndex] += logLuminanceSums[threadIndex + stride];
        GroupMemoryBarrierWithGroupSync();
    }

    if (threadIndex == 0)
        stepBuffer[0] = exp(logLuminanceSums[0] / float(MEASURE_GRID * MEASURE_GRID));
}

// auto exposure, step 2: scales the image so its average luminance becomes mid grey times the effect's amount
[shader("compute")]
[numthreads(8, 8, 1)]
void cApplyExposure(uint3 pixel : SV_DispatchThreadID)
{
    uint width;
    uint height;
    destinationImage.GetDimensions(width, height);
    if (pixel.x >= width || pixel.y >= height)
        return;

    const float2 uv = (float2(pixel.xy) + 0.5f) / float2(width, height);
    const float4 color = sourceImage.SampleLevel(uv, 0.0f);
    const float exposure = clamp(0.18f * postProcess.amount / max(stepBuffer[0], 0.0001f), 0.25f, 4.0f);
    destinationImage[pixel.xy] = float4(color.rgb * exposure, color.a);
}
