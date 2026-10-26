struct VSOutput
{
    float4 Pos : SV_Position;
    float2 UV;
}

[shader("vertex")]
VSOutput main(uint vertexIndex: SV_VertexID)
{
    VSOutput output;
    if (vertexIndex == 0)
        output.Pos = float4(-1.0, -1.0, 0.0, 1.0);
    else if (vertexIndex == 1)
        output.Pos = float4(-1.0, 3.0, 0.0, 1.0);
    else
        output.Pos = float4(3.0, -1.0, 0.0, 1.0);
    output.UV = output.Pos.xy * 0.5 + 0.5;
    return output;
}