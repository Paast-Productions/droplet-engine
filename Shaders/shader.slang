struct VSInput 
{
    float3 inPosition;
    float3 inColor;
    float3 inNormal;
    float2 inTexCoord;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float4 fragNormal;
    float3 fragColor;
    float2 fragTexCoord;
};

struct UniformBuffer
{
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};
ConstantBuffer<UniformBuffer> ubo;

[shader("vertex")]
VSOutput vertMain(VSInput input) {
    VSOutput output;
    output.pos = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.inPosition, 1.0))));
    output.fragTexCoord = input.inTexCoord;
    output.fragNormal = float4(input.inNormal, 1.0);
    output.fragColor = input.inColor;
    return output;
}

Sampler2D texture;

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET {

    float3 color = texture.Sample(float2(vertIn.fragTexCoord.x, -vertIn.fragTexCoord.y)).rgb;

    // Normalize vectors
    float3 N = normalize(vertIn.fragNormal.xyz);
    float3 L = float3(0.0,0.0,1.0);
    float3 V = float3(0.0,0.0,-1.0);

    // Ambient component
    float3 ambient = 0.1f * color;

    // Diffuse component (Lambert)
    float diff = max(dot(N, L), 0.0f);
    float3 diffuse = diff * color;

    // Blinn-Phong Specular component
    float3 H = normalize(L + V); // Halfway vector
    float specAngle = max(dot(N, H), 0.0f);
    float spec = pow(specAngle, 100);
    float3 specular = spec;

    return float4(color, 0.0);
}