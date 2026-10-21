struct PushConstants
{
    float4x4 world;
    float32_t time;
    uint32_t sampler;
};

[[vk::push_constant]] PushConstants pc;

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

layout (set = 0, binding = 0) ConstantBuffer<UniformBuffer> vp;

[shader("vertex")]
VSOutput main(VSInput input) {
    VSOutput output;
    float4 pos = float4(input.inPosition.x, input.inPosition.y, input.inPosition.z, 1.0);
    output.pos = mul(vp.proj, mul(vp.view, mul(pc.world, pos)));
    output.fragTexCoord = input.inTexCoord;
    output.fragNormal = float4(input.inNormal, 1.0);
    return output;
}