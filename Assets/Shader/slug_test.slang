struct CameraUniform
{
    float4x4 view;
    float4x4 projection;
    float aspectRatio;
};

[[vk::binding(0, 0)]]
ConstantBuffer<CameraUniform> camera;

[[vk::binding(1, 0)]]
Texture2D<float4> curvesTexture;

[[vk::binding(2, 0)]]
[[vk::image_format("rg16ui")]]
Texture2D<uint4> bandsTexture;

struct GlyphInstance
{
    float4 scaleBias;
    float4 glyphBandScale;
    uint4 bandMaxTexCoords;
};

[[vk::binding(3, 0)]]
StructuredBuffer<GlyphInstance> glyphInstances;

struct VertexOutput
{
    float4 position : SV_Position;
    float2 texCoords : TEXCOORD0;
    nointerpolation uint instanceID : TEXCOORD1;
}


[shader("vertex")]
VertexOutput slugVertMain(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    GlyphInstance instance = glyphInstances[instanceID];

    static const float2 positions[6] = 
    {
        // First triangle
        float2(-0.5f, -0.5f), // Bottom-left
        float2( 0.5f, -0.5f), // Bottom-right
        float2( 0.5f,  0.5f), // Top-right

        // Second triangle
        float2(-0.5f, -0.5f), // Bottom-left
        float2( 0.5f,  0.5f), // Top-right
        float2(-0.5f,  0.5f)  // Top-left
    };
    
    float fixedAdvance = 0.25;
    float xOffset = (float)instanceID * fixedAdvance;

    
    VertexOutput output;
    
    // output.position = mul(camera.projection, mul(camera.view, float4((positions[vertexID].x / camera.aspectRatio) + fixedAdvance, positions[vertexID].y, 0.5f, 1.0f)));
    float2 position = positions[vertexID] * instance.scaleBias.xy + instance.scaleBias.zw;
    position.x /= camera.aspectRatio;
    output.position = mul(camera.projection, mul(camera.view, float4(position, 0.5f, 1.0f)));
    output.texCoords = positions[vertexID] + float2(0.5f, 0.5f);
    output.instanceID = instanceID;
    return output;
}

static const float epsilon = 0.0001;
float TraceRayCurveH(float2 p1, float2 p2, float2 p3, float pixelsPerEm)
{
    if (max(max(p1.x, p2.x), p3.x) * pixelsPerEm < -0.5)
    {
        // The right-most point of the curve is to the left of the current fragment.
        // Because the curves are sorted, we can exit.
        return 0.0;
    }

    uint code = (0x2E74U >> (((p1.y > 0.0) ? 2U : 0U) + ((p2.y > 0.0) ? 4U : 0U) + ((p3.y > 0.0) ? 8U : 0U))) & 3U;
    if (code == 0U)
    {
        return 0.0;
    }

    float2 a = p1 - p2 * 2.0 + p3;
    float2 b = p1 - p2;
    float c = p1.y;
    float ayr = 1.0 / a.y;
    float d = sqrt(max(b.y * b.y - a.y * c, 0.0));
    float t1 = (b.y - d) * ayr;
    float t2 = (b.y + d) * ayr;

    if (abs(a.y) < epsilon)
    {
        t1 = t2 = c / (2.0 * b.y);
    }

    float coverage = 0.0;

    if ((code & 1U) != 0U)
    {
        float x1 = (a.x * t1 - b.x * 2.0) * t1 + p1.x;
        float c = clamp(x1 * pixelsPerEm + 0.5, 0.0, 1.0);
        coverage += c;
    }

    if (code > 1U)
    {
        float x2 = (a.x * t2 - b.x * 2.0) * t2 + p1.x;
        float c = clamp(x2 * pixelsPerEm + 0.5, 0.0, 1.0);
        coverage -= c;
    }

    return coverage;
}

float TraceRayBandH(uint2 bandData, float pixelsPerEm, float2 texCoords, float2 glyphScale)
{
    float coverage = 0.0;

    for (uint curve = 0U; curve < bandData.x; ++curve)
    {
        uint curveOffset = bandData.y + curve;
        int2 curveLoc = bandsTexture.Load(
            int3(int(curveOffset & 0xFFFu), int(curveOffset >> 12u), 0)
        ).xy;

        float4 p12 = curvesTexture.Load(
            int3(int2(curveLoc), 0)
        ) / float4(glyphScale, glyphScale) - float4(texCoords, texCoords);

        float2 p3 = curvesTexture.Load(
            int3(int2(curveLoc) + int2(1, 0), 0)
        ).xy / glyphScale - texCoords;

        coverage += TraceRayCurveH(p12.xy, p12.zw, p3.xy, pixelsPerEm);
    }
    return coverage;
}

float TraceRayBandV(uint2 bandData, float pixelsPerEm, float2 texCoords, float2 glyphScale)
{
    float coverage = 0.0;

    for (uint curve = 0U; curve < bandData.x; ++curve)
    {
        uint curveOffset = bandData.y + curve;
        int2 curveLoc = bandsTexture.Load(
            int3(int(curveOffset & 0xFFFu), int(curveOffset >> 12u), 0)
        ).xy;

        float4 p12 = curvesTexture.Load(
            int3(int2(curveLoc), 0)
        ) / float4(glyphScale, glyphScale) - float4(texCoords, texCoords);

        float2 p3 = curvesTexture.Load(
            int3(int2(curveLoc) + int2(1, 0), 0)
        ).xy / glyphScale - texCoords;

        coverage += TraceRayCurveH(p12.yx, p12.wz, p3.yx, pixelsPerEm);
    }
    return coverage;
}

[shader("fragment")]
float4 slugFragMain(VertexOutput input) : SV_TARGET
{
    GlyphInstance instance = glyphInstances[input.instanceID];

    float2 glyphScale = instance.glyphBandScale.xy;
    float2 bandScale = instance.glyphBandScale.zw;

    float2 bandMax = instance.bandMaxTexCoords.xy;
    float2 bandsTexCoords = instance.bandMaxTexCoords.zw;

    float2 pixelsPerEm = 1.0 / fwidth(input.texCoords);

    uint2 bandIndex = min(
        uint2(max(input.texCoords * bandScale, float2(0.0, 0.0))),
        bandMax
    );

    uint hBandOffset = bandsTexCoords.y * 4096U + bandsTexCoords.x + bandIndex.y;
    uint2 hBandData = bandsTexture.Load(
        int3(int(hBandOffset & 0xFFFu), int(hBandOffset >> 12u), 0)
    ).xy;

    uint vBandOffset = bandsTexCoords.y * 4096U + bandsTexCoords.x + bandMax.y + 1U + bandIndex.x;
    uint2 vBandData = bandsTexture.Load(
        int3(int(vBandOffset & 0xFFFu), int(vBandOffset >> 12u), 0)
    ).xy;

    float coverageX = TraceRayBandH(hBandData, pixelsPerEm.x, input.texCoords, glyphScale);
    float coverageY = TraceRayBandV(vBandData, pixelsPerEm.y, input.texCoords, glyphScale);

    coverageX = min(abs(coverageX), 1.0);
    coverageY = min(abs(coverageY), 1.0);
    
    return float4(1.0, 1.0, 1.0, (coverageX + coverageY) * 0.5);
}
