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

layout(set = 2, binding = 0) Sampler2D textureArr[];

float Hash31(float3 p)
{
    p = frac(p * 0.1031);
    p += dot(p, p.xyz + 33.33);
    return frac((p.x + p.y) * p.z);
}

float ValueNoise(float3 p)
{
    float3 i = floor(p);
    float3 f = frac(p);

    float3 u = f * f * (3.0 - 2.0 * f);

    float n000 = Hash31(i + float3(0, 0, 0));
    float n100 = Hash31(i + float3(1, 0, 0));
    float n010 = Hash31(i + float3(0, 1, 0));
    float n110 = Hash31(i + float3(1, 1, 0));

    float n001 = Hash31(i + float3(0, 0, 1));
    float n101 = Hash31(i + float3(1, 0, 1));
    float n011 = Hash31(i + float3(0, 1, 1));
    float n111 = Hash31(i + float3(1, 1, 1));

    float x00 = lerp(n000, n100, u.x);
    float x10 = lerp(n010, n110, u.x);
    float x01 = lerp(n001, n101, u.x);
    float x11 = lerp(n011, n111, u.x);

    float y0 = lerp(x00, x10, u.y);
    float y1 = lerp(x01, x11, u.y);

    return lerp(y0, y1, u.z);
}

float FractalNoise(float3 p)
{
    float noise = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    float totalAmplitude = 0.0;

    for (int i = 0; i < 3; i++)
    {
        float sample = ValueNoise(p * frequency);

        noise += sample * amplitude;
        totalAmplitude += amplitude;

        frequency *= 2.0;

        amplitude *= 0.5;
    }

    return noise / totalAmplitude;
}

[shader("fragment")]
float4 main(FSInput vertIn) : SV_TARGET
{
    float height = vertIn.fragTexCoord.y;
    float underside = vertIn.fragTexCoord.x;

    float3 pos = vertIn.wPos.xyz;
    float3 animatedPos = pos * 0.65 + float3(0.0, pc.time * 0.2, 0.0);

    float fracNoise = FractalNoise(animatedPos);
    float density = smoothstep(0.0, 0.85, fracNoise);
    float alpha = 0.27 + density * 0.25;

    float3 color = float3(0.2, 0.6, 0.4);

    return float4(color, alpha * 0.5);
}
