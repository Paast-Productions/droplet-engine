
[[vk::image_format("rgba8")]] RWTexture2D<float4> ImageOutput;
[[vk::binding(0, 1)]] Sampler2D ImageInput;
 
//  Slang/HLSL do not have GLSL's (not)equal functions
bool3 notEqual(float3 a, float3 b) {
    return bool3(a.x != b.x, a.y != b.y, a.z != b.z);
}

[shader("compute")]
[numthreads(8, 8, 1)]
void main(uint3 globalInvocationID: SV_DispatchThreadID, uniform float counter)
{
    float4 v = ImageInput.Load(int3(globalInvocationID.xy, 0));

    // Increase intensity over time until the cell dies.
    if (any(notEqual(v.rgb, float3(0.0))))
        v.w = max(v.w, counter);
    else
        v.w = 0.0;

    ImageOutput[globalInvocationID.xy] = v; 
}