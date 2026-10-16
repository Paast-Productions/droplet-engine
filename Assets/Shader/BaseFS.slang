struct FSInput 
{
    float4 pos : SV_Position;
    float4 fragNormal;
    float2 fragTexCoord;
};

layout(set = 0, binding = 1) Sampler2D texture;

[shader("fragment")]
float4 main(FSInput vertIn) : SV_TARGET {

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

    return float4(1.0, 1.0, 1.0, 0.0);
}
