static float2 positions[3] = float2[](
    float2(0.0, -0.5),
    float2(0.5, 0.5),
    float2(-0.5, 0.5));

static float3 colors[3] = float3[](
    float3(1.0, 0.0, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, 0.0, 1.0));

struct VertexOutput
{
    float3 color;
    float4 sv_position : SV_Position;
};

[shader("vertex")]
VertexOutput vertexMain(uint vertexID: SV_VertexID)
{
    VertexOutput output;
    output.sv_position = float4(positions[vertexID], 0.0f, 1.0f);
    output.color = colors[vertexID];
    return output;
}

[shader("fragment")]
float4 fragmentMain(VertexOutput input) : SV_Target0
{
    float3 color = input.color;
    return float4(color, 1.0);
}
