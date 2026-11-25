// The renderer's shader for P3C4 dynamic geometry (debug lines and shapes): vertex colour, unlit

struct PushConstants
{
    float4x4 world;
    float4 cameraDir;
    float4 cameraPos;
    float time;
    uint textureID; // index into textureArr
};
[[vk::push_constant]] PushConstants pc;

struct ViewProjection
{
    float4x4 view;
    float4x4 proj;
};
[[vk::binding(0, 0)]] ConstantBuffer<ViewProjection> viewProjection;

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
    output.position = mul(viewProjection.proj, mul(viewProjection.view, mul(pc.world, float4(input.position, 1.0))));
    output.color = input.color;
    return output;
}

[shader("fragment")]
float4 fMain(VertexOutput input) : SV_Target
{
    return input.color;
}
