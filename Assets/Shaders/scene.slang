// The renderer's built-in scene shader, used when the default shaders are missing. Production's binding interface:
// push constants, set 0 view and projection, set 2 the texture array. Vertex layout P3N3UV2

struct PushConstants
{
    float4x4 world;
    float4 cameraDir;
    float4 cameraPos;
    float time;
    uint textureID; // index into textureArr
};
[[vk::push_constant]] PushConstants pc;

struct ViewProjection
{
    float4x4 view;
    float4x4 proj;
};
[[vk::binding(0, 0)]] ConstantBuffer<ViewProjection> viewProjection;

// set 1 is reserved for storage buffers (lights)
[[vk::binding(0, 2)]] Sampler2D textureArr[];

struct VertexInput
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD0;
};

struct VertexOutput
{
    float4 position    : SV_Position;
    float3 worldNormal : NORMAL;
    float2 uv          : TEXCOORD0;
};

[shader("vertex")]
VertexOutput vMain(VertexInput input)
{
    VertexOutput output;
    float4 world = mul(pc.world, float4(input.position, 1.0));
    output.position = mul(viewProjection.proj, mul(viewProjection.view, world));
    output.worldNormal = mul((float3x3)pc.world, input.normal); // fine for uniform scale
    output.uv = input.uv;
    return output;
}

[shader("fragment")]
float4 fMain(VertexOutput input) : SV_Target
{
    // one fixed directional light plus ambient, so the shape reads even with a flat texture
    float3 normal = normalize(input.worldNormal);
    float3 toLight = normalize(float3(0.4, 0.8, 0.6));
    float diffuse = max(dot(normal, toLight), 0.0);

    float3 albedo = textureArr[pc.textureID].Sample(input.uv).rgb;
    return float4(albedo * (0.25 + 0.75 * diffuse), 1.0);
}
