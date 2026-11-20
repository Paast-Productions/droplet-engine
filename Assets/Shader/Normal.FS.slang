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

[shader("fragment")]
float4 main(FSInput vertIn) : SV_TARGET
{
    float a = length(vertIn.pos) + length(vertIn.wPos) + length(vertIn.fragTexCoord);
    float4 color = float4((normalize(vertIn.fragNormal.xyz) + 1.0.rrr) / 2, 1.0);
    return color;
}
