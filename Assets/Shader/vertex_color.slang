// The renderer's shader for P3C4 dynamic geometry (debug lines and shapes): vertex colour, unlit

struct PushConstants
{
    float4x4 viewProj;   // projection * view of the camera being rendered
    float4x4 model;      // object to world
};
[[vk::push_constant]] ConstantBuffer<PushConstants> pc;

struct VertexInput
{
    float3 position : POSITION;
    float4 color    : COLOR;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float4 color    : COLOR;
};

[shader("vertex")]
VertexOutput vMain(VertexInput input)
{
    VertexOutput output;
    output.position = mul(pc.viewProj, mul(pc.model, float4(input.position, 1.0)));
    output.color = input.color;
    return output;
}

[shader("fragment")]
float4 fMain(VertexOutput input) : SV_Target
{
    return input.color;
}
