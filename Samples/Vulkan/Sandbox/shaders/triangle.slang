[shader("vertex")]
float4 vertexMain(uint vertexID : SV_VertexID) : SV_Position
{
    float2 positions[3] =
    {
        float2( 0.0, -0.5),
        float2( 0.5,  0.5),
        float2(-0.5,  0.5)
    };

    return float4(positions[vertexID], 0.0, 1.0);
}

[shader("fragment")]
float4 fragmentMain() : SV_Target0
{
    return float4(0.0, 1.0, 0.0, 1.0);
}