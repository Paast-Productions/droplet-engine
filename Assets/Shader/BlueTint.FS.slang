struct PushConstants
{
    float4x4 world;
    float32_t time;
    uint32_t sampler;
};

[[vk::push_constant]] PushConstants pc;

struct FSInput 
{
    float4 pos : SV_Position;
    float4 fragNormal;
    float2 fragTexCoord;
};

layout(set = 0, binding = 1) Sampler2D texture[];

[shader("fragment")]
float4 main(FSInput vertIn) : SV_TARGET 
{
    float3 colour = texture[pc.sampler].Sample(float2(vertIn.fragTexCoord.x, -vertIn.fragTexCoord.y)).rgb;
    
    float3 blueTintColour = colour * float3(0.65f, 0.65f, 1.0f);
    
    return float4(blueTintColour, 1.0);
}
