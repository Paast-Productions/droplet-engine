#language slang 2026

namespace PostProcess
{
// helper enum for pp
public enum Effect {
    Grayscale,
    Contrast,
    Invert,
    Passthrough,
    BlurHorizontal,   // compute shader only (postprocess.cs.slang); amount: radius in pixels
    BlurVertical,     // compute shader only
    MeasureLuminance, // compute shader only (cMeasureLuminance)
    ApplyExposure     // compute shader only (cApplyExposure); amount: target brightness relative to mid grey
}

// parameters for the post-processing
public struct Parameters
{
    Effect effect;
    float amount;
};

// remove chrominance and leave only the 'brightness' of the color
// TODO: account for different colour scales than sRGB
// url: https://www.johndcook.com/blog/2009/08/24/algorithms-convert-color-grayscale/
public float grayscale(float3 color)
{
    const float3 luminance = float3(0.2126, 0.7152, 0.0722);
    return dot(color, luminance);
}

// push contrasts from the 'center' by a factor of amount
public float3 contrast(float3 color, float amount)
{
    return saturate((color.rgb - 0.5) * amount + 0.5);
}

// invert colours
public float3 invert(float3 color) { return float3(1.0f - color);}

// the colour after a per-pixel effect selected in parameters; alpha is kept. Other effects pass through
public float4 apply(float4 color, Parameters parameters)
{
    switch (parameters.effect)
    {
    case Effect::Grayscale:
    {
        const float luminance = grayscale(color.rgb);

        return float4(luminance.xxx, color.a);
    }

    case Effect::Contrast:
    {
        const float3 contrasted = contrast(color.rgb, parameters.amount);

        return float4(contrasted, color.a);
    }

    // invert all colours
    case Effect::Invert:
    {
        return float4(invert(color.rgb), color.a);
    }

    case Effect::Passthrough:
    default:
        return color;
    }
}

}