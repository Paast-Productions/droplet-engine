// Droplet's Base.VS/Base.FS for the sandbox's scene interface

struct PushConstants
{
    float4x4 viewProj;   // projection * view of the camera being rendered
    float4x4 model;      // object to world
};
[[vk::push_constant]] ConstantBuffer<PushConstants> pc;

// set 0, binding 0: the object's texture
[[vk::binding(0, 0)]] Sampler2D baseColor;

// set 1, binding 0: the view's uniforms (Vulkan::SceneViewUniforms)
struct ViewUniforms
{
    float4 cameraPosition;
    float4 cameraForward;
    float time;
};
[[vk::binding(0, 1)]] ConstantBuffer<ViewUniforms> view;

struct VertexInput
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD0;
};

struct VertexOutput
{
    float4 position      : SV_Position;
    float4 worldPosition : POSITION1;
    float4 worldNormal   : NORMAL;
    float2 uv            : TEXCOORD0;
};

[shader("vertex")]
VertexOutput vMain(VertexInput input)
{
    VertexOutput output;
    output.worldPosition = mul(pc.model, float4(input.position, 1.0));
    output.position = mul(pc.viewProj, output.worldPosition);
    output.worldNormal = normalize(mul(pc.model, float4(input.normal, 0.0)));
    output.uv = input.uv;
    return output;
}

[shader("fragment")]
float4 fMain(VertexOutput input) : SV_Target
{
    // V flipped, as Base.FS samples it (the sampler repeats)
    float4 color = baseColor.Sample(float2(input.uv.x, -input.uv.y));
    clip(color.w - 0.5);

    // brighter where the surface faces the camera
    float3 toCamera = normalize(view.cameraPosition.xyz - input.worldPosition.xyz);
    float viewDotNormal = dot(input.worldNormal.xyz, toCamera);
    color.xyz *= saturate((viewDotNormal + 1.0) / 2.0);
    return color;
}
