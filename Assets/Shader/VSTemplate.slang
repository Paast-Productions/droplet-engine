//Values that are quick to receive in the shader but have limited space, 128-256 bytes
//Most values are unused in this shader but are accessible
struct PushConstants
{
    float4x4 world;
    float4 cameraDir;
    float4 cameraPos;
    float32_t time;
    uint32_t sampler;
};

[[vk::push_constant]] PushConstants pc;

//Definition of input values, must match binding description sent to rendering pipeline
struct VSInput 
{
    float3 inPosition;
    float3 inNormal;
    float2 inTexCoord;
};

//Definition of output values
struct VSOutput
{
    float4 pos : SV_Position;
    float4 wPos;
    float4 fragNormal;
    float2 fragTexCoord;
};

//Pretty much a constant buffer, with bindless it could be used as an array, however, here we are only using the first stored buffer
struct UniformBuffer
{
    float4x4 view;
    float4x4 proj;
};

layout(set = 0, binding = 0) ConstantBuffer<UniformBuffer> vp;

[shader("vertex")]
VSOutput main(VSInput input) {
    VSOutput output;

    //Basic world view projection multiplication
    float4 pos = float4(input.inPosition.xyz, 1.0);

    //Store world position for fragment shader
    output.wPos = mul(pc.world, pos);

    //Store screen space position for fragment shader
    output.pos = mul(vp.proj, mul(vp.view, output.wPos));

    //Store transformed normal for fragment shader
    output.fragNormal = mul(pc.world, float4(input.inNormal, 0.0));

    //Store UV coordinates for fragment shader
    output.fragTexCoord = input.inTexCoord;

    return output;
}