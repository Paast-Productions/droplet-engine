struct CameraUniform
{
    float4x4 view;
    float4x4 projection;
};

[[vk::binding(0, 0)]]
ConstantBuffer<CameraUniform> camera;

struct DebugVertexInput
{
    [[vk::location(0)]] float3 position;
    [[vk::location(1)]] float4 color;
};

struct DebugVertexOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float4 color;
};

[shader("vertex")]
DebugVertexOutput vertMain(DebugVertexInput input)
{
    DebugVertexOutput output;
    output.position = mul(camera.projection, mul(camera.view, float4(input.position, 1.0)));
    output.color = input.color;
    return output;
}

[shader("fragment")]
float4 fragMain(DebugVertexOutput input) : SV_TARGET
{
    return input.color;
}
