struct PushConstants
{
    float4x4 world;
    float4 cameraDir;
    float4 cameraPos;
    float32_t time;
    uint32_t sampler;
};

[[vk::push_constant]] PushConstants pc;

struct VSInput 
{
    float3 inPosition;
    float3 inNormal;
    float2 inTexCoord;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float4 wPos;
    float4 fragNormal;
    float2 fragTexCoord;
};

struct UniformBuffer
{
    float4x4 view;
    float4x4 proj;
};

layout (set = 0, binding = 0) ConstantBuffer<UniformBuffer> vp;

static const float WAVE_AMPLITUDE = 0.5;
static const float WAVE_SCALE = 1.25;
static const float NORMAL_EPSILON = 0.25;

float2 hash22(float2 p)
{
    float3 p3 = frac(float3(p.xyx) * float3(0.1031, 0.1030, 0.0973));
    p3 += dot(p3, p3.yzx + 33.33);
    return frac((p3.xx + p3.yz) * p3.zy);
}

// Returns distances to the nearest (F1) and second nearest (F2) animated feature point.
float2 voronoi(float2 p, float time)
{
    float2 cell = floor(p);
    float2 local = frac(p);
    float f1 = 8.0;
    float f2 = 8.0;

    for (int j = -1; j <= 1; j++)
    {
        for (int i = -1; i <= 1; i++)
        {
            float2 offset = float2(float(i), float(j));
            float2 jitter = hash22(cell + offset);
            // Feature points drift around their cell so the pattern moves over time.
            jitter = 0.5 + 0.5 * sin(time * 0.8 + 6.2831 * jitter);
            float2 r = offset + jitter - local;
            float d = dot(r, r);

            if (d < f1)
            {
                f2 = f1;
                f1 = d;
            }
            else if (d < f2)
            {
                f2 = d;
            }
        }
    }

    return float2(sqrt(f1), sqrt(f2));
}

// Height of the ocean surface at local plane coordinate p, in [-1, 1] range.
float waveHeight(float2 p, float time)
{
    float2 v = voronoi(p, time);

    // Rounded crests from F1, plus sharper caustic-like edges from F2 - F1.
    float crests = 1.0 - v.x;
    float edges = 1.0 - saturate((v.y - v.x) * 4.0);

    float swell = sin(dot(p, float2(0.8, 0.6)) * 0.9 + time * 1.3);

    return 0.5 * crests + 0.3 * edges * edges + 0.2 * swell;
}

[shader("vertex")]
VSOutput main(VSInput input) 
{
    VSOutput output;

    // Waves are evaluated in world space, so the surface stays fixed as objects move.
    float3 wp = mul(pc.world, float4(input.inPosition, 1.0)).xyz;

    // Y is up: the water surface lies in the world XZ plane and is displaced along Y.
    float2 q = wp.xz * WAVE_SCALE;
    float t = pc.time;
    float h = waveHeight(q, t);

    // Central differences of the height field, converted back to world XZ units.
    float dhdx = (waveHeight(q + float2(NORMAL_EPSILON, 0.0), t) - waveHeight(q - float2(NORMAL_EPSILON, 0.0), t)) / (2.0 * NORMAL_EPSILON);
    float dhdz = (waveHeight(q + float2(0.0, NORMAL_EPSILON), t) - waveHeight(q - float2(0.0, NORMAL_EPSILON), t)) / (2.0 * NORMAL_EPSILON);
    float2 slope = float2(dhdx, dhdz) * WAVE_SCALE * WAVE_AMPLITUDE;

    // Surface y = H(x, z) has normal (-dH/dx, 1, -dH/dz).
    float3 worldNormal = normalize(float3(-slope.x, 1.0, -slope.y));

    output.wPos = float4(wp.x, wp.y + WAVE_AMPLITUDE * h, wp.z, 1.0);
    output.pos = mul(vp.proj, mul(vp.view, output.wPos));
    output.fragNormal = float4(worldNormal, 0.0);
    output.fragTexCoord = input.inTexCoord;

    return output;
}
