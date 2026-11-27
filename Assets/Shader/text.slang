struct GlyphInstance
{
    float4 scaleBias;
    float4 glyphBandScale;
    uint4 bandMaxTexCoords;
};

struct TextPushConstants
{
    float4x4 model;
    float4 color;

    uint textureID;
    uint firstInstance;
    uint2 padding;
};

[[vk::push_constant]]
ConstantBuffer<TextPushConstants> textPushConstants;

struct ViewProjection
{
    float4x4 view;
    float4x4 proj;
}

[[vk::binding(0, 0)]]
ConstantBuffer<ViewProjection> camera;

[[vk::binding(0, 1)]]
StructuredBuffer<GlyphInstance> glyphInstances;

[[vk::binding(0, 2)]]
Sampler2D textureArr[];

static const uint ROW_WIDTH = 4096u;
static const float epsilon = 0.0001f;

struct VertexOutput
{
    float4 position : SV_Position;
    float2 texCoords : TEXCOORD0;

    nointerpolation uint glyphIndex : TEXCOORD1;
}


[shader("vertex")]
VertexOutput vMain(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
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

    uint glyphIndex = textPushConstants.firstInstance + instanceID;
    GlyphInstance glyph = glyphInstances[glyphIndex];
    
    float2 localPosition = positions[vertexID] * glyph.scaleBias.xy + glyph.scaleBias.zw;

    float4 worldPosition = mul(textPushConstants.model, float4(localPosition, 0.0f, 1.0f));
    
    VertexOutput output;
    
    output.position = mul(camera.proj, mul(camera.view, worldPosition));
    output.texCoords = positions[vertexID] + float2(0.5f);
    output.glyphIndex = glyphIndex;
    return output;
}

float4 LoadBand(uint index, uint bandStartRow)
{
    return textureArr[textPushConstants.textureID].Load(
        int3(
            int(index % ROW_WIDTH),
            int(index / ROW_WIDTH + bandStartRow),
            0
        )
    );
}

float TraceRayCurveH(float2 p1, float2 p2, float2 p3, float pixelsPerEm)
{
    // Slug lookup table
    uint code =
        (0x2E74U >>
            (((p1.y > 0.0) ? 2U : 0U) +
             ((p2.y > 0.0) ? 4U : 0U) +
             ((p3.y > 0.0) ? 8U : 0U))) & 3U;

    if (code == 0U)
    {
        return 0.0f;
    }

    float2 a = p1 - p2 * 2.0f + p3;
    float2 b = p1 - p2;
    float c = p1.y;

    float d = sqrt(max(b.y * b.y - a.y * c, 0.0));

    float t1, t2;

    if (abs(a.y) < epsilon)
    {
        t1 = t2 = c / (2.0 * b.y);
    }
    else
    {
        t1 = (b.y - d) / a.y;
        t2 = (b.y + d) / a.y;
    }

    float coverage = 0.0f;

    if ((code & 1U) != 0U)
    {
        float x1 = (a.x * t1 - b.x * 2.0f) * t1 + p1.x;
        coverage += clamp(x1 * pixelsPerEm + 0.5f, 0.0f, 1.0f);
    }

    if ((code & 2U) != 0U)
    {
        float x2 = (a.x * t2 - b.x * 2.0f) * t2 + p1.x;
        coverage -= clamp(x2 * pixelsPerEm + 0.5f, 0.0f, 1.0f);
    }

    return coverage;
}

float TraceRayBand(uint2 bandData, float pixelsPerEm, float2 texCoords, float2 glyphScale, uint2 rows, bool vertical)
{
    float coverage = 0.0f;

    for (uint curve = 0U; curve < bandData.x; ++curve)
    {
        int2 curveLoc = int2(LoadBand(bandData.y + curve, rows.y).xy);

        curveLoc.y += int(rows.x);

        float4 p12 = textureArr[textPushConstants.textureID].Load(int3(curveLoc, 0))
                        / float4(glyphScale, glyphScale) - float4(texCoords, texCoords);

        float2 p3 = textureArr[textPushConstants.textureID].Load(int3(curveLoc + int2(1, 0), 0)).xy / glyphScale - texCoords;


        // The curves are sorted. If the statement below is true, that means we can exit
        // because no more curves will hit.
        const float maxAxis = vertical ?   max(max(p12.y, p12.w), p3.y)
                                         : max(max(p12.x, p12.z), p3.x);

        if (maxAxis * pixelsPerEm < -0.5f)
            break;

        if (vertical)
        {
            coverage += TraceRayCurveH(p12.yx, p12.wz, p3.yx, pixelsPerEm);
        }
        else
        {
            coverage += TraceRayCurveH(p12.xy, p12.zw, p3.xy, pixelsPerEm);
        }
    }
    return coverage;
}


[shader("fragment")]
float4 fMain(VertexOutput input) : SV_TARGET
{
    GlyphInstance glyph = glyphInstances[input.glyphIndex];

    if (glyph.glyphBandScale.x == 0.0f)
    {
        float2 edge = min(input.texCoords, 1.0f - input.texCoords);
        float distanceToEdge = min(edge.x, edge.y);

        float border = 1.0f - smoothstep(0.03f, 0.05f, distanceToEdge);

        return float4(
            textPushConstants.color.rgb,
            textPushConstants.color.a * border
        );
    }

    uint2 rows = uint2(textureArr[textPushConstants.textureID].Load(int3(0, 0, 0)).xy);

    float2 glyphScale = glyph.glyphBandScale.xy;
    float2 bandScale = glyph.glyphBandScale.zw;

    uint2 bandMax = glyph.bandMaxTexCoords.xy;
    uint2 bandsTexCoords = glyph.bandMaxTexCoords.zw;

    uint headerIndex = bandsTexCoords.y * ROW_WIDTH + bandsTexCoords.x;

    float2 pixelsPerEm = 1.0f / fwidth(input.texCoords);

    uint2 bandIndex = min(
        uint2(max(input.texCoords * bandScale, float2(0.0f))),
        bandMax
    );

    uint2 horizontalBand = uint2(LoadBand(headerIndex + bandIndex.y, rows.y).xy);
    uint2 verticalBand = uint2(LoadBand(headerIndex + bandMax.y + 1U + bandIndex.x, rows.y).xy);

    float coverageX = TraceRayBand(horizontalBand, pixelsPerEm.x, input.texCoords, glyphScale, rows, false);
    float coverageY = TraceRayBand(verticalBand, pixelsPerEm.y, input.texCoords, glyphScale, rows, true);

    coverageX = min(abs(coverageX), 1.0f);
    coverageY = min(abs(coverageY), 1.0f);

    float glyphCoverage = (coverageX + coverageY) * 0.5f;
    return float4(textPushConstants.color.rgb, textPushConstants.color.a * glyphCoverage);
    
}