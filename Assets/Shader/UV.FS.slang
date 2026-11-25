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
    float4 color = float4(vertIn.fragTexCoord, 0.0, 1.0);
    return color;
}
