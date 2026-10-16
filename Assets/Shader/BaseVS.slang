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
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};

layout (set = 0, binding = 0) StructuredBuffer<UniformBuffer> ubo;

[shader("vertex")]
VSOutput main(VSInput input) {
    VSOutput output;
    //output.pos = mul(ubo[0].proj, mul(ubo[0].view, mul(ubo[0].model, float4(input.inPosition, 1.0))));
    output.pos = float4(input.inPosition, 1.0);
    output.fragTexCoord = input.inTexCoord;
    output.fragNormal = float4(input.inNormal, 1.0);
    return output;
}