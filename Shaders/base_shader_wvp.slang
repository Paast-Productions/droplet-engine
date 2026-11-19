
struct VSInput
{
    float2 inPosition;
    float3 inColor;
};

struct UniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};
ConstantBuffer<UniformBuffer> ubo;

struct VSOutput{
    float4 pos : SV_Position;
    float3 color;
};

[shader("vertex")]
VSOutput vMain(VSInput input){
    VSOutput output;
    output.pos = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.inPosition, 0.0, 1.0))));
    output.color = input.inColor;
    return output;
}

[shader("fragment")]
float4 fMain(VSOutput vsOut) : SV_Target 
{
    return float4(vsOut.color, 1.0);
}
    