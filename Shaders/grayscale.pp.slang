Texture2D<float4> sceneTexture;
SamplerState sceneSampler;

struct PSInput
{
    float4 position : SV_Position;
    float2 uv       : TEXCOORD0;
};

[shader("fragment")]
float4 main(PSInput input) : SV_Target
{
    float4 color = sceneTexture.Sample(sceneSampler, input.uv);
    const float3 scale = float3(0.2126, 0.7152, 0.0722);
    float gray = dot(color.rgb, scale);

    return float4(gray, gray, gray, color.a);
}