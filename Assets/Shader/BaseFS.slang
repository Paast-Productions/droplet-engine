struct FSInput 
{
    float4 pos : SV_Position;
    float4 fragNormal;
    float2 fragTexCoord;
};

layout(set = 0, binding = 1) Sampler2D texture;

[shader("fragment")]
float4 main(FSInput vertIn) : SV_TARGET {

    return float4(1.0, 1.0, 1.0, 1.0);
}
