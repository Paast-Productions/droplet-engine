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
    float3 inpos = input.inPosition;
    float4 wPos = mul(pc.world, float4(inpos, 1.0));

    float4 pos = float4(inpos.x, inpos.y, inpos.z + 0.2 * cos(100 * pc.time + wPos.x + wPos.z) * saturate(inpos.y), 1.0);
    output.wPos = mul(pc.world, pos);
    output.pos = mul(vp.proj, mul(vp.view,  output.wPos));
    output.fragTexCoord = input.inTexCoord;
    output.fragNormal = mul(pc.world, float4(input.inNormal, 0.0));

    return output;
}