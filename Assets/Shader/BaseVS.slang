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

struct PushConstants
{
    uint32_t unibuffer;
    uint32_t sampler;
};

layout (set = 0, binding = 0) ConstantBuffer<UniformBuffer> ubo;

[shader("vertex")]
VSOutput main(VSInput input) {
    VSOutput output;
    output.pos = mul(ubo.proj, mul(ubo.view, float4(input.inPosition, 1.0)));
    output.fragTexCoord = input.inTexCoord;
    output.fragNormal = float4(input.inNormal, 1.0);
    return output;
}