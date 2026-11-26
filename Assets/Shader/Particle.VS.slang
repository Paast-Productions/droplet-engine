// Particle emitter vertex stage: places each particle's centre in world space. Particle.GS expands it into a quad.

struct PushConstants
{
    float4x4 world;
    float4 cameraDir;
    float4 cameraPos;
    float time;
    uint textureID;
};
[[vk::push_constant]] PushConstants pc;

struct VSInput
{
    [[vk::location(0)]] float3 position : POSITION;
    [[vk::location(1)]] float4 color : COLOR;
    [[vk::location(2)]] float size : SIZE;
    [[vk::location(3)]] float rotation : ROTATION;
};

struct VSOutput
{
    [[vk::location(0)]] float3 center : CENTER;
    [[vk::location(1)]] float4 color : COLOR;
    [[vk::location(2)]] float size : SIZE;
    [[vk::location(3)]] float rotation : ROTATION;
};

[shader("vertex")]
VSOutput main(VSInput input)
{
    VSOutput output;
    output.center = mul(pc.world, float4(input.position, 1.0)).xyz;
    // the emitter's scale applies to the particle size (uniform scale assumed)
    output.size = input.size * length(mul(pc.world, float4(1.0, 0.0, 0.0, 0.0)).xyz);
    output.color = input.color;
    output.rotation = input.rotation;
    return output;
}
