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

layout(set = 2, binding = 0) Sampler2D textureArr[];

[shader("fragment")]
float4 main(FSInput vertIn) : SV_TARGET 
{
    float3 colour = textureArr[pc.sampler].Sample(float2(vertIn.fragTexCoord.x, -vertIn.fragTexCoord.y)).rgb;
    
    return float4(colour, 1.0);
}
