// Particle emitter fragment stage: the emitter's texture tinted by the particle colour

struct PushConstants
{
    float4x4 world;
    float4 cameraDir;
    float4 cameraPos;
    float time;
    uint textureID;
};
[[vk::push_constant]] PushConstants pc;

layout(set = 2, binding = 0) Sampler2D textureArr[];

struct FSInput
{
    [[vk::location(0)]] float2 uv : TEXCOORD0;
    [[vk::location(1)]] float4 color : COLOR;
};

[shader("fragment")]
float4 main(FSInput input) : SV_TARGET
{
    return textureArr[pc.textureID].Sample(input.uv) * input.color;
}
