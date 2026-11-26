// Particle emitter geometry stage: expands each particle point into a quad in the view plane, so it always faces the camera

struct ViewProjection
{
    float4x4 view;
    float4x4 proj;
};
[[vk::binding(0, 0)]] ConstantBuffer<ViewProjection> viewProjection;

struct GSInput
{
    [[vk::location(0)]] float3 center : CENTER;
    [[vk::location(1)]] float4 color : COLOR;
    [[vk::location(2)]] float size : SIZE;
    [[vk::location(3)]] float rotation : ROTATION;
};

struct GSOutput
{
    float4 position : SV_Position;
    [[vk::location(0)]] float2 uv : TEXCOORD0;
    [[vk::location(1)]] float4 color : COLOR;
};

[shader("geometry")]
[maxvertexcount(4)]
void main(point GSInput input[1], inout TriangleStream<GSOutput> stream)
{
    const GSInput particle = input[0];
    const float3 centerView = mul(viewProjection.view, float4(particle.center, 1.0)).xyz;
    const float halfSize = particle.size * 0.5;
    const float s = sin(particle.rotation);
    const float c = cos(particle.rotation);

    // a strip in the order (-,-), (+,-), (-,+), (+,+)
    for (int i = 0; i < 4; i++)
    {
        const float2 corner = float2((i & 1) != 0 ? 1.0 : -1.0, (i & 2) != 0 ? 1.0 : -1.0);
        const float2 rotated = float2(c * corner.x - s * corner.y, s * corner.x + c * corner.y);

        GSOutput vertex;
        vertex.position = mul(viewProjection.proj, float4(centerView + float3(rotated * halfSize, 0.0), 1.0));
        // image top (v = 0) is the quad's top
        vertex.uv = float2(corner.x * 0.5 + 0.5, 0.5 - corner.y * 0.5);
        vertex.color = particle.color;
        stream.Append(vertex);
    }
    stream.RestartStrip();
}
