struct VSOutput
{
    float4 Pos : SV_Position;
    float2 UV;
}

Sampler2D in_image;

float4 convert_color(float4 value)
{
    float gray = dot(value.rgb, float3(0.3, 0.6, 0.1));
    return float4(lerp(float3(gray), value.rgb, value.a), 1.0);
}

[shader("fragment")]
float4 main(VSOutput input)
{
    return convert_color(in_image.SampleLevel(input.UV, 0.0));
}