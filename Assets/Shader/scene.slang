// Temporary scene shader for Graphics::ScenePass. Vertex layout: Vertex::P3N3UV2 (position, normal, uv).
// Matrices arrive column-major from glm; the shader compiler is configured for column-major, so mul(M, v) is M * v.
// Entry points must be called vMain and fMain (the pipeline looks them up by name).

struct PushConstants
{
    float4x4 viewProj;   // projection * view of the camera being rendered
    float4x4 model;      // object to world
};

[[vk::push_constant]] ConstantBuffer<PushConstants> pc;

// set 0, binding 0: the object's texture (a loaded texture, or another camera's render target)
[[vk::binding(0, 0)]] Sampler2D baseColor;

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
    float4 world = mul(pc.model, float4(input.position, 1.0));
    output.position = mul(pc.viewProj, world);
    output.worldNormal = mul((float3x3)pc.model, input.normal); // fine for uniform scale
    output.uv = input.uv;
    return output;
}

[shader("fragment")]
float4 fMain(VertexOutput input) : SV_Target
{
    // One fixed directional light plus ambient, so the shape reads even with a flat texture.
    float3 normal = normalize(input.worldNormal);
    float3 toLight = normalize(float3(0.4, 0.8, 0.6));
    float diffuse = max(dot(normal, toLight), 0.0);

    float3 albedo = baseColor.Sample(input.uv).rgb;
    return float4(albedo * (0.25 + 0.75 * diffuse), 1.0);
}
