struct CameraUniform
{
    float4x4 view;
    float4x4 projection;
    float aspectRatio;
};

[[vk::binding(0, 0)]]
ConstantBuffer<CameraUniform> camera;

struct IconVertexOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float4 color;
    [[vk::location(1)]] float2 texCoord;
};

[shader("vertex")]
IconVertexOutput slugVertMain(uint vertexID : SV_VertexID)
{
    static const float2 positions[6] = 
    {
        // First triangle
        float2(-0.5f, -0.5f), // Bottom-left
        float2( 0.5f, -0.5f), // Bottom-right
        float2( 0.5f,  0.5f), // Top-right

        // Second triangle
        float2(-0.5f, -0.5f), // Bottom-left
        float2( 0.5f,  0.5f), // Top-right
        float2(-0.5f,  0.5f)  // Top-left
    };
    IconVertexOutput output;
    output.position = float4(positions[vertexID].x / camera.aspectRatio, positions[vertexID].y, 0.5f, 1.0f);
    output.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    output.texCoord = float2(0.0f, 0.0f);
    return output;
}

[shader("fragment")]
float4 slugFragMain(IconVertexOutput input) : SV_TARGET
{
    return input.color;
}
