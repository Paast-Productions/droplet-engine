struct CameraUniform
{
    float4x4 view;
    float4x4 projection;
};

[[vk::binding(0, 0)]]
ConstantBuffer<CameraUniform> camera;

struct IconVertexInput
{
    [[vk::location(0)]] float3 position;
    [[vk::location(1)]] float4 color;
    [[vk::location(2)]] float2 texCoord;
};

struct IconVertexOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float4 color;
    [[vk::location(1)]] float2 texCoord;
};

[shader("vertex")]
IconVertexOutput iconVertMain(IconVertexInput input)
{
    IconVertexOutput output;
    output.position = mul(camera.projection, mul(camera.view, float4(input.position, 1.0)));
    output.color = input.color;
    output.texCoord = input.texCoord;
    return output;
}

[shader("fragment")]
float4 iconFragMain(IconVertexOutput input) : SV_TARGET
{
    return input.color;
}
