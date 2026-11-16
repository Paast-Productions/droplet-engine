struct PushConstants
{
    float4x4 world;
    float4 cameraDir;
    float4 cameraPos;
    float32_t time;
    uint32_t sampler;
};

[[vk::push_constant]] PushConstants pc;

struct FSInput 
{
    float4 pos : SV_Position;
    float4 wPos;
    float4 fragNormal;
    float2 fragTexCoord;
};

layout(set = 2, binding = 0) Sampler2D textureArr[];

float3 lanternOffset = normalize(pc.cameraDir).xyz * 0.5;

[shader("fragment")]
float4 main(FSInput vertIn) : SV_TARGET
{
    float4 color = textureArr[pc.sampler].Sample(float2(vertIn.fragTexCoord.x, -vertIn.fragTexCoord.y));
    
    clip(color.w - 0.5);

    float3 lightPos = pc.cameraPos.xyz;

    float3 lightToFrag = normalize(lightPos - vertIn.wPos.xyz);

    float3 camToFrag = normalize(pc.cameraPos.xyz - vertIn.wPos.xyz);

    float3 lightColor = float3(1.0, 1.0, 0.8);

    float viewDotNormal = (dot(vertIn.fragNormal.xyz, camToFrag));
    color.xyz *= saturate((viewDotNormal + 1.0.rrr)) * lightColor;

    float attenuation = saturate(1.0 - length(lightToFrag) / 5.0);

    return float4(color) * attenuation * attenuation;
}
