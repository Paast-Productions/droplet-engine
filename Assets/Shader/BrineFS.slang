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

float2 hash22(float2 p)
{
    float3 p3 = frac(float3(p.xyx) * float3(0.1031, 0.1030, 0.0973));
    p3 += dot(p3, p3.yzx + 33.33);
    return frac((p3.xx + p3.yz) * p3.zy);
}

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
    float height = round(vertIn.fragTexCoord.y * 3.1) * (1.0 / 3.0);
    float underside = vertIn.fragTexCoord.x;

    float heightAlphaOffset = ((1.0 - height) * 1.2 + 0.25);
    float heightCurrentStrength = (height * 0.5 + 0.5);

    float2 currentDir = hash22(float2(height * 9.79491, height * 3.518));
    currentDir -= float2(0.5, 0.5);
    currentDir = normalize(currentDir);

    float3 pos = vertIn.wPos.xyz;
    float3 animatedPos = pos * 0.65 + float3(currentDir.x * pc.time * heightCurrentStrength * 0.5, 0.0, currentDir.y * pc.time * heightCurrentStrength * 0.5);

    float fracNoise = FractalNoise(animatedPos * float3(1.0, 0.0, 1.0));
    float density = smoothstep(0.0, 0.85, fracNoise);
    float noiseAlpha = 0.27 + density * 0.25;

    float3 color = float3(0.3, 0.6, 0.45);
    float alpha = (noiseAlpha * noiseAlpha * 2.0) * heightAlphaOffset;
    //float alpha = noiseAlpha * heightAlphaOffset;

    // if (vertIn.fragTexCoord.y < 0.9)
    //if (vertIn.fragTexCoord.y < 0.4 || vertIn.fragTexCoord.y > 0.6)
    //if (vertIn.fragTexCoord.y > 0.1)
    if (false)
    {
        clip(-1);
    }

    return float4(color, alpha);
}
