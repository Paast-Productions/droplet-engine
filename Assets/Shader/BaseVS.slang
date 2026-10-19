struct VSInput 
{
    float3 inPosition;
    float3 inNormal;
    float2 inTexCoord;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float4 fragNormal;
    float2 fragTexCoord;
};

struct UniformBuffer
{
    float4x4 view;
    float4x4 proj;
};

struct WorldBuffer
{
    float4x4 world;
}

struct PushConstants
{
    uint32_t unibuffer;
    uint32_t sampler;
};

[[vk::push_constant]] WorldBuffer pc;

layout (set = 0, binding = 0) ConstantBuffer<UniformBuffer> vp;

[shader("vertex")]
VSOutput main(VSInput input) {
    VSOutput output;
    output.pos = mul(vp.proj, mul(vp.view, mul(pc.world, float4(input.inPosition, 1.0))));
    output.fragTexCoord = input.inTexCoord;
    output.fragNormal = float4(input.inNormal, 1.0);
    return output;
}