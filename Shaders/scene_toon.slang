// Toon scene shader: the scene material interface of scene.slang (vertex layout P3N3UV2, set 0 binding 0 base colour,
// push constants viewProj + model) with the light quantised into three bands.
// Entry points must be called vMain and fMain.

struct PushConstants
{
    float4x4 viewProj;   // projection * view of the camera being rendered
    float4x4 model;      // object to world
};

[[vk::push_constant]] ConstantBuffer<PushConstants> pc;

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
    // same light as scene.slang, quantised into three bands; the band edges are blended over a narrow range so a flat
    // face fades to the next band as it turns instead of switching at once
    float3 normal = normalize(input.worldNormal);
    float3 toLight = normalize(float3(0.4, 0.8, 0.6));
    float diffuse = max(dot(normal, toLight), 0.0);

    const float EDGE_WIDTH = 0.08;
    float banded = 0.3
                 + 0.3 * smoothstep(0.33 - EDGE_WIDTH, 0.33 + EDGE_WIDTH, diffuse)
                 + 0.4 * smoothstep(0.66 - EDGE_WIDTH, 0.66 + EDGE_WIDTH, diffuse);

    float3 albedo = baseColor.Sample(input.uv).rgb;
    return float4(albedo * (0.2 + 0.8 * banded), 1.0);
}

// less smooth
// [shader("fragment")]
// float4 fMain(VertexOutput input) : SV_Target
// {
//     // same light as scene.slang, quantised into three bands
//     float3 normal = normalize(input.worldNormal);
//     float3 toLight = normalize(float3(0.4, 0.8, 0.6));
//     float diffuse = max(dot(normal, toLight), 0.0);
//     float banded = diffuse > 0.66 ? 1.0 : (diffuse > 0.33 ? 0.6 : 0.3);
// 
//     float3 albedo = baseColor.Sample(input.uv).rgb;
//     return float4(albedo * (0.2 + 0.8 * banded), 1.0);
// }
