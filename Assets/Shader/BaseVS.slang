struct PushConstants
{
    float4x4 world;
    float4 cameraDir;
    float4 cameraPos;
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
    float4 wPos;
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
VSOutput main(VSInput input) 
{
    VSOutput output;
    float4 pos = float4(input.inPosition.xyz, 1.0);
    output.wPos = mul(pc.world, pos);
    output.pos = mul(vp.proj, mul(vp.view, output.wPos));
    output.fragNormal = mul(pc.world, float4(input.inNormal, 0.0));
    output.fragTexCoord = input.inTexCoord;
    output.fragNormal = mul(pc.world, float4(input.inNormal, 0.0));
    return output;
}