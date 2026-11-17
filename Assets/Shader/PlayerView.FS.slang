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



[shader("fragment")]
float4 main(FSInput vertIn) : SV_TARGET
{
    float4 color = textureArr[pc.sampler].Sample(float2(vertIn.fragTexCoord.x, -vertIn.fragTexCoord.y));
    
    clip(color.w - 0.5);

    float3 lightPos = pc.cameraPos.xyz;

    float3 fragToLight = normalize(lightPos - vertIn.wPos.xyz);

    float3 fragToCam = normalize(pc.cameraPos.xyz - vertIn.wPos.xyz);

    float3 H = normalize(fragToLight + fragToCam);

    float3 lightColor = float3(1.0, 1.0, 0.90);

    float hDotNormal = dot(vertIn.fragNormal.xyz, H);

    float attenuation = 5.0 / (length(lightPos - vertIn.wPos.xyz) * length(lightPos - vertIn.wPos.xyz));

    color.xyz *= saturate((hDotNormal.rrr)) * saturate(attenuation) * lightColor;

    return float4(color);
}
