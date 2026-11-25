#language slang 2026
// without this we would have to designate every member variable as public to have it visible
// outside this module

module shared_types;

// different vertex configurations
namespace Vertex
{
public struct IN_2P3C
{
    float2 inPosition;
    float3 inColor;
};

public struct OUT_2P3C
{
    float4 pos : SV_Position;
    float3 color : TEXCOORD0;
};

public struct IN_2PUV
{
    float2 pos;
    float2 uv;
};

public struct OUT_2PUV
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};
}