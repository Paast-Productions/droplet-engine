struct PushConstants
{
    float4x4 world;
    float4 cameraDir;
    float4 cameraPos;
    float32_t time;
    uint32_t sampler;
};

[[vk::push_constant]] PushConstants pc;

struct FSInput
{
    float4 pos : SV_Position;
    float4 wPos;
    float4 fragNormal;
    float2 fragTexCoord;
};

static const float3 BRINE_SHALLOW = float3(0.10, 0.38, 0.32);
static const float3 BRINE_DEEP = float3(0.02, 0.12, 0.13);
static const float3 BRINE_GLINT = float3(0.65, 0.92, 0.80);

float hash13(float3 p)
{
    p = frac(p * 0.1031);
    p += dot(p, p.zyx + 31.32);
    return frac((p.x + p.y) * p.z);
}

float valueNoise(float3 p)
{
    float3 i = floor(p);
    float3 f = frac(p);
    f = f * f * (3.0 - 2.0 * f);

    float n000 = hash13(i);
    float n100 = hash13(i + float3(1, 0, 0));
    float n010 = hash13(i + float3(0, 1, 0));
    float n110 = hash13(i + float3(1, 1, 0));
    float n001 = hash13(i + float3(0, 0, 1));
    float n101 = hash13(i + float3(1, 0, 1));
    float n011 = hash13(i + float3(0, 1, 1));
    float n111 = hash13(i + float3(1, 1, 1));

    float x00 = lerp(n000, n100, f.x);
    float x10 = lerp(n010, n110, f.x);
    float x01 = lerp(n001, n101, f.x);
    float x11 = lerp(n011, n111, f.x);

    float y0 = lerp(x00, x10, f.y);
    float y1 = lerp(x01, x11, f.y);

    return lerp(y0, y1, f.z);
}

// Fractal noise in [0, 1] built from four octaves of value noise.
float fbm(float3 p)
{
    float sum = 0.0;
    float amp = 0.5;
    for (int i = 0; i < 4; i++)
    {
        sum += amp * valueNoise(p);
        p = p * 2.03 + float3(17.1, 4.3, 9.7);
        amp *= 0.5;
    }
    return sum / 0.9375;
}

static const float FLOW_DISTANCE = 0.8;
static const float DIRECTION_RATE = 0.2;

// Random unit direction for a given layer and direction "cell" (changes every cell).
float3 flowDir(float layerId, float cell)
{
    float3 s = float3(layerId * 13.7 + cell * 0.731, layerId * 5.3 + cell * 1.37, layerId * 2.9 + cell * 2.11 + 4.0);
    return normalize(float3(hash13(s) * 2.0 - 1.0, hash13(s + 17.3) * 2.0 - 1.0, hash13(s + 31.9) * 2.0 - 1.0) + 1e-4);
}

// Two offset flows, each advancing along its own direction and resetting when its cell wraps.
// Their weights sum to 1 and go to 0 at the wrap, so direction changes are seamless.
float flowNoise(float3 p, float layerId, float tt)
{
    float ph1 = frac(tt);
    float c1 = floor(tt);
    float ph2 = frac(tt + 0.5);
    float c2 = floor(tt + 0.5);

    float w1 = 1.0 - abs(2.0 * ph1 - 1.0);
    float w2 = 1.0 - w1;

    float3 o1 = flowDir(layerId, c1) * ph1 * FLOW_DISTANCE;
    float3 o2 = flowDir(layerId, c2) * ph2 * FLOW_DISTANCE;

    return w1 * fbm(p + o1) + w2 * fbm(p + o2);
}

[shader("fragment")]
float4 main(FSInput vertIn) : SV_TARGET
{
    // uv.y encodes the layer: 0 = bottom, 0.5 = middle, 1 = top.
    // layer is 1 at the bottom and 0 at the top.
    float layer = saturate(1.0 - vertIn.fragTexCoord.y);
    float layerId = round(vertIn.fragTexCoord.y * 2.0);

    // Layers are offset so they don't match each other; each layer flows in its own random direction.
    float tt = pc.time * DIRECTION_RATE;
    float3 layerOffset = float3(layer * 7.3, layer * 3.1, layer * 5.7);
    float3 p = vertIn.wPos.xyz * 1.5 + layerOffset;

    float n1 = flowNoise(p, layerId, tt);
    float n2 = flowNoise(p * 2.7 + 13.1, layerId, tt);

    // Opacity: subtle at the top (~0.08), mostly opaque at the bottom (~0.9), modulated by noise.
    float baseAlpha = lerp(0.08, 0.9, layer);
    float alpha = baseAlpha * lerp(0.6, 1.3, n1);

    // Thicken the view at grazing angles.
    float3 normal = normalize(vertIn.fragNormal.xyz);
    float3 camToFrag = normalize(pc.cameraPos.xyz - vertIn.wPos.xyz);
    float facing = abs(dot(normal, camToFrag));
    float grazing = pow(1.0 - facing, 3.0);
    alpha = saturate(alpha + grazing * 0.15);

    // Brine colour: lighter and greener near the surface, darker and denser at depth.
    float3 baseColor = lerp(BRINE_SHALLOW, BRINE_DEEP, layer);
    float3 color = baseColor * lerp(0.8, 1.2, n1);

    // Sparse bright glints where the second noise layer peaks.
    float glint = smoothstep(0.68, 0.82, n2) * (1.0 - layer * 0.5);
    color += BRINE_GLINT * glint * 0.35;

    return float4(saturate(color), saturate(alpha));
}
