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
    [[vk::location(2)]] float2 size : SIZE;
    [[vk::location(3)]] float rotation : ROTATION;
    [[vk::location(4)]] float2 billboardOffset : BILLBOARDOFFSET;
    [[vk::location(5)]] float4 uvRect : TEXCOORD1;
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
    const float2 halfSize = particle.size * 0.5;
    const float s = sin(particle.rotation);
    const float c = cos(particle.rotation);

    // a strip in the order (-,-), (+,-), (-,+), (+,+)
    for (int i = 0; i < 4; i++)
    {
        const float2 corner = float2((i & 1) != 0 ? 1.0 : -1.0, (i & 2) != 0 ? 1.0 : -1.0);
        // scale by width and height first so the rotation turns the rectangle rather than stretching it
        const float2 offset = corner * halfSize;
        const float2 rotated = float2(c * offset.x - s * offset.y, s * offset.x + c * offset.y);
        // the billboard offset is in the camera plane, so it does not turn with the rotation
        const float2 shifted = rotated + particle.billboardOffset;

        GSOutput vertex;
        vertex.position = mul(viewProjection.proj, float4(centerView + float3(shifted, 0.0), 1.0));
        // image top (v = 0) is the quad's top; the particle's tile rect selects the part of the image shown
        const float2 unitUV = float2(corner.x * 0.5 + 0.5, 0.5 - corner.y * 0.5);
        vertex.uv = lerp(particle.uvRect.xy, particle.uvRect.zw, unitUV);
        vertex.color = particle.color;
        stream.Append(vertex);
    }
    stream.RestartStrip();
}
