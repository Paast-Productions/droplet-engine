//Values that are quick to receive in the shader but have limited space, 128-256 bytes
//pc.sampler is used for finding the texture in the Sampler2D array
struct PushConstants
{
    float4x4 world;
    float4 cameraDir;
    float4 cameraPos;
    float32_t time;
    uint32_t sampler;
};

[[vk::push_constant]] PushConstants pc;

//Definition of shader input values, must match vertex shader otherwise incorrect data will be read
struct FSInput 
{
    float4 pos : SV_Position;
    float4 wPos;
    float4 fragNormal;
    float2 fragTexCoord;
};

//Normally you would add a sampler without the "[]" but in bindless all used textures are bound to the same descriptor
//To access a specific texture, the CPU sends the index through the PushConstants sampler.
layout(set = 0, binding = 1) Sampler2D texture[];

[shader("fragment")]
float4 main(FSInput vertIn) : SV_TARGET 
{
    //Basic shading calculations
    float3 color = texture[pc.sampler].Sample(float2(vertIn.fragTexCoord.x, -vertIn.fragTexCoord.y)).rgb;

    float3 camToFrag = normalize(pc.cameraPos.xyz - vertIn.wPos.xyz);

    float viewDotNormal = abs(dot(vertIn.fragNormal.xyz, camToFrag));
    color *= saturate((viewDotNormal + 1.0.rrr) / 2.0);

    return float4(color, 1.0);
}
