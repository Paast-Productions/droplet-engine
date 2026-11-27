struct PushConstants
{
    float4x4 world;
    float4 cameraDir;
    float4 cameraPos;
    float32_t time;
    uint32_t sampler;
    uint32_t lightCount;
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

struct LightData
{
    float4 positionAndRange;
    float4 colorAndIntensity;
}

layout(set = 1, binding = 0) StructuredBuffer<LightData> lights;

[shader("fragment")]
float4 main(FSInput vertIn) : SV_TARGET
{
    float4 color = textureArr[pc.sampler].Sample(float2(vertIn.fragTexCoord.x, -vertIn.fragTexCoord.y));
    
    clip(color.w - 0.5);

    //float3 camToFrag = normalize(pc.cameraPos.xyz - vertIn.wPos.xyz);

    //float viewDotNormal = (dot(vertIn.fragNormal.xyz, camToFrag));
    //color.xyz *= saturate((viewDotNormal + 1.0.rrr) / 2.0);
    //return float4(color);

    float3 totalLighting = float3(0.0, 0.0, 0.0);

    for (int i = 0; i < pc.lightCount; i++)
    {
        LightData light = lights[i];

        
        float3 lightPos = light.positionAndRange.xyz;
        float3 lightColor = light.colorAndIntensity.xyz;
        
        float intensity = light.colorAndIntensity.w;
        float range = light.positionAndRange.w;
        
        float3 toLight = lightPos - vertIn.wPos.xyz;
        float distanceToLight = length(toLight);
        
        float3 L = toLight / max(distanceToLight, 0.0001);
        float3 N = normalize(vertIn.fragNormal.xyz);
        
        //float3 L = normalize(lightPos - vertIn.wPos.xyz);
        
        float diffuse = saturate(dot(N,L));
        
        float attenuation = 1.0 / max(distanceToLight * distanceToLight, 0.01);
        
        float rangeFade = saturate(1.0 - distanceToLight / max(range, 0.0001));
        
        float3 lighting = diffuse * lightColor * intensity * attenuation * rangeFade; 
        
        //return float4(lights[0].colorAndIntensity.xyz, 1.0);
        totalLighting += lighting;
    }

    color.xyz *= 0.1 + totalLighting;

    return color;
}
