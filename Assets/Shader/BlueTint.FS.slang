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

layout(set = 0, binding = 1) Sampler2D texture[];

[shader("fragment")]
float4 main(FSInput vertIn) : SV_TARGET
{
    float4 color = texture[pc.sampler].Sample(float2(vertIn.fragTexCoord.x, -vertIn.fragTexCoord.y));

    clip(color.w - 0.5);

    float3 camToFrag = normalize(pc.cameraPos.xyz - vertIn.wPos.xyz);

    float viewDotNormal = (dot(vertIn.fragNormal.xyz, camToFrag));
    color.xyz *= saturate((viewDotNormal + 1.0.rrr) / 2.0);

    float4 blueTintColour = lerp(color, float4(0.00f, 0.00f, 0.3f, 1.0), saturate(length(pc.cameraPos.xyz - vertIn.wPos.xyz) / 50));

    return blueTintColour;
}
