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

layout(set = 0, binding = 1) Sampler2D texture;

[shader("fragment")]
float4 main(FSInput vertIn) : SV_TARGET
{
    float3 colour = texture.Sample(float2(vertIn.fragTexCoord.x + pc.time, -vertIn.fragTexCoord.y + pc.time)).rgb;
    
    return float4(colour, 1.0);
}
