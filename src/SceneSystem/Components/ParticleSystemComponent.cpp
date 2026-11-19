#include "ParticleSystemComponent.hpp"

#include <SceneSystem/Node.hpp>
#include <SceneSystem/Scene.hpp>
#include <Time.hpp>
#include <resource/ResourceManager.hpp>
#include <Editor/ResourceSlot.hpp>
#include <Editor/ImGuiCurve.hpp>
#include <Graphics/DebugDraw.hpp>
#include <Math/Bounds/Sphere.hpp>
#include <cmath>
#include <algorithm>

using namespace Droplet;
using namespace Droplet::Scene;
using namespace Droplet::Graphics;

ParticleSystemComponent::ParticleSystemComponent()
{
    m_particles.reserve(m_maxParticles);
}

ParticleSystemComponent::ScalarCurve ParticleSystemComponent::MakeDefaultCurve(float value)
{
    ScalarCurve curve;
    curve.type = Math::Curves::CurveType::Linear;
    curve.points = {
        { glm::vec2(0.0f, value), glm::vec2(0.0f, value), glm::vec2(0.0f, value) },
        { glm::vec2(1.0f, value), glm::vec2(1.0f, value), glm::vec2(1.0f, value) }
    };
    return curve;
}

void ParticleSystemComponent::StartImpl()
{
    SetRenderable();
    if (m_playOnAwake)
        Play();
}

void ParticleSystemComponent::Play()
{
    m_paused = false;
    m_playing = true;
}

void ParticleSystemComponent::Pause()
{
    m_paused = true;
    m_playing = false;
}

void ParticleSystemComponent::Stop(bool clearParticles)
{
    m_playing = false;
    m_paused = false;
    m_systemTime = 0.0f;
    m_delayElapsed = 0.0f;
    m_emissionAccumulator = 0.0f;
    ResetBurstTriggers();
    if (clearParticles)
        m_particles.clear();
}

void ParticleSystemComponent::SetPlaying(bool playing)
{
    if (playing)
        Play();
    else
        Pause();
}

void ParticleSystemComponent::Emit(uint32_t count)
{
    if (m_particles.capacity() < m_maxParticles)
        m_particles.reserve(m_maxParticles);
    const uint32_t available = m_maxParticles - static_cast<uint32_t>(m_particles.size());
    for (uint32_t i = 0; i < count && i < available; ++i)
        SpawnParticle();
}

void ParticleSystemComponent::SetDuration(float duration) { m_duration = std::max(0.001f, duration); }
void ParticleSystemComponent::SetLooping(bool looping) { m_looping = looping; }
void ParticleSystemComponent::SetPlayOnAwake(bool playOnAwake) { m_playOnAwake = playOnAwake; }
void ParticleSystemComponent::SetStartDelay(float startDelay) { m_startDelay = std::max(0.0f, startDelay); }
void ParticleSystemComponent::SetMaxParticles(uint32_t maxParticles)
{
    m_maxParticles = std::max(1u, maxParticles);
    m_particles.reserve(m_maxParticles);
    if (m_particles.size() > m_maxParticles)
        m_particles.resize(m_maxParticles);
}
void ParticleSystemComponent::SetEmissionRate(float particlesPerSecond) { m_emissionRate = std::max(0.0f, particlesPerSecond); }
void ParticleSystemComponent::SetLifetimeRange(const FloatRange &range) { m_lifetimeRange = { std::max(0.001f, range.min), std::max(0.001f, range.max) }; }
void ParticleSystemComponent::SetSpeedRange(const FloatRange &range) { m_speedRange = range; }
void ParticleSystemComponent::SetSizeRange(const FloatRange &range) { m_sizeRange = { std::max(0.0f, range.min), std::max(0.0f, range.max) }; }
void ParticleSystemComponent::SetRotationRange(const FloatRange &range) { m_rotationRange = range; }
void ParticleSystemComponent::SetAngularVelocityRange(const FloatRange &range) { m_angularVelocityRange = range; }
void ParticleSystemComponent::SetStartColorRange(const ColorRange &range) { m_startColorRange = range; }
void ParticleSystemComponent::SetGravityModifier(float gravityModifier) { m_gravityModifier = gravityModifier; }
void ParticleSystemComponent::SetEmissionShape(EmissionShape shape) { m_emissionShape = shape; }
void ParticleSystemComponent::SetSimulationSpace(SimulationSpace space) { m_simulationSpace = space; }
void ParticleSystemComponent::SetShapeRadius(float radius) { m_shapeRadius = std::max(0.0f, radius); }
void ParticleSystemComponent::SetShapeBoxSize(const glm::vec3 &size) { m_shapeBoxSize = glm::max(size, glm::vec3(0.0f)); }
void ParticleSystemComponent::SetConeAngle(float angleDegrees) { m_coneAngle = std::clamp(angleDegrees, 0.0f, 180.0f); }
void ParticleSystemComponent::AddBurst(float time, uint32_t count)
{
    m_bursts.push_back({ std::max(0.0f, time), std::max(1u, count), false });
    std::sort(m_bursts.begin(), m_bursts.end(), [](const Burst &a, const Burst &b) { return a.time < b.time; });
}
void ParticleSystemComponent::ClearBursts()
{
    m_bursts.clear();
}

void ParticleSystemComponent::SetTextureGUID(GUID guid)
{
    m_texture = ResourceManager::GetInstance().LoadResource<Texture2DResource>(guid);
    if (MetaEntry entry; ResourceManager::GetInstance().GetResourceMetaEntry(guid, entry))
        m_textureDisplayName = entry.displayName;
}

float ParticleSystemComponent::RandomFloat(float min, float max)
{
    if (max < min)
        std::swap(min, max);
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(m_random);
}

glm::vec3 ParticleSystemComponent::RandomUnitSphere()
{
    const float z = RandomFloat(-1.0f, 1.0f);
    const float angle = RandomFloat(0.0f, glm::two_pi<float>());
    const float radius = std::sqrt(std::max(0.0f, 1.0f - z * z));
    return glm::vec3(radius * std::cos(angle), radius * std::sin(angle), z);
}

glm::vec3 ParticleSystemComponent::RandomDirectionForEmission()
{
    switch (m_emissionShape)
    {
    case EmissionShape::Sphere:
    case EmissionShape::Hemisphere:
    case EmissionShape::Circle:
        return RandomUnitSphere();
    case EmissionShape::Cone:
    {
        const float coneAngle = glm::radians(m_coneAngle);
        const float z = std::cos(RandomFloat(0.0f, coneAngle));
        const float angle = RandomFloat(0.0f, glm::two_pi<float>());
        const float planar = std::sin(std::acos(z));
        return glm::normalize(glm::vec3(planar * std::cos(angle), planar * std::sin(angle), z));
    }
    case EmissionShape::Box:
        return RandomUnitSphere();
    default:
        return RandomUnitSphere();
    }
}

void ParticleSystemComponent::SpawnParticle()
{
    if (m_particles.size() >= m_maxParticles)
        return;

    glm::vec3 localPosition(0.0f);
    glm::vec3 direction = RandomUnitSphere();
    switch (m_emissionShape)
    {
    case EmissionShape::Sphere:
        localPosition = RandomUnitSphere() * m_shapeRadius;
        direction = glm::normalize(localPosition);
        break;
    case EmissionShape::Hemisphere:
        localPosition = RandomUnitSphere() * m_shapeRadius;
        localPosition.y = std::abs(localPosition.y);
        direction = glm::normalize(localPosition);
        break;
    case EmissionShape::Box:
        localPosition = {
            RandomFloat(-m_shapeBoxSize.x, m_shapeBoxSize.x) * 0.5f,
            RandomFloat(-m_shapeBoxSize.y, m_shapeBoxSize.y) * 0.5f,
            RandomFloat(-m_shapeBoxSize.z, m_shapeBoxSize.z) * 0.5f
        };
        break;
    case EmissionShape::Cone:
        localPosition = glm::vec3(0.0f);
        direction = RandomDirectionForEmission();
        break;
    case EmissionShape::Circle:
        direction = RandomUnitSphere();
        localPosition = glm::normalize(glm::vec3(direction.x, direction.y, 0.0f)) * RandomFloat(0.0f, m_shapeRadius);
        break;
    case EmissionShape::Point:
    default:
        break;
    }

    glm::vec3 worldPosition = localPosition;
    if (GetOwner())
        worldPosition = m_simulationSpace == SimulationSpace::Local
            ? GetOwner()->GetTransform().GetMatrix(Transform::Space::World) * glm::vec4(localPosition, 1.0f)
            : GetOwner()->GetTransform().GetPosition(Transform::Space::World) + localPosition;

    const float lifetime = RandomFloat(m_lifetimeRange.min, m_lifetimeRange.max);
    const float speed = RandomFloat(m_speedRange.min, m_speedRange.max);
    const float red = RandomFloat(m_startColorRange.min.r, m_startColorRange.max.r);
    const float green = RandomFloat(m_startColorRange.min.g, m_startColorRange.max.g);
    const float blue = RandomFloat(m_startColorRange.min.b, m_startColorRange.max.b);
    const float alpha = RandomFloat(m_startColorRange.min.a, m_startColorRange.max.a);

    Particle particle;
    particle.position = worldPosition;
    particle.velocity = glm::normalize(direction) * speed;
    particle.baseSize = RandomFloat(m_sizeRange.min, m_sizeRange.max);
    particle.size = particle.baseSize;
    particle.rotation = glm::radians(RandomFloat(m_rotationRange.min, m_rotationRange.max));
    particle.angularVelocity = glm::radians(RandomFloat(m_angularVelocityRange.min, m_angularVelocityRange.max));
    particle.color = glm::vec4(red, green, blue, alpha);
    particle.lifetime = lifetime;
    m_particles.push_back(particle);
}

float ParticleSystemComponent::EvaluateCurve(const ScalarCurve &curve, float normalizedTime) const
{
    if (curve.points.empty())
        return 0.0f;
    if (curve.points.size() == 1)
        return curve.points.front().position.y;

    normalizedTime = std::clamp(normalizedTime, 0.0f, 1.0f);
    std::size_t right = 1;
    while (right < curve.points.size() && curve.points[right].position.x < normalizedTime)
        ++right;
    if (right >= curve.points.size())
        return curve.points.back().position.y;

    const auto &left = curve.points[right - 1];
    const auto &righ = curve.points[right];
    const float span = righ.position.x - left.position.x;
    if (span <= 0.0001f)
        return righ.position.y;

    float t = std::clamp((normalizedTime - left.position.x) / span, 0.0f, 1.0f);
    if (curve.type == Math::Curves::CurveType::BezierCubic)
        return glm::mix(glm::mix(glm::mix(left.position, left.controlPoint2, t), glm::mix(left.controlPoint2, righ.controlPoint1, t), t),
            glm::mix(glm::mix(left.controlPoint2, righ.controlPoint1, t), glm::mix(righ.controlPoint1, righ.position, t), t), t).y;
    if (curve.type == Math::Curves::CurveType::BezierQuadratic)
    {
        const glm::vec2 p0 = left.position;
        const glm::vec2 p1 = left.controlPoint1;
        const glm::vec2 p2 = righ.position;
        return (1.0f - t) * (1.0f - t) * p0.y + 2.0f * (1.0f - t) * t * p1.y + t * t * p2.y;
    }
    return glm::mix(left.position.y, righ.position.y, t);
}

void ParticleSystemComponent::UpdateParticles(float deltaTime)
{
    const glm::vec3 gravity(0.0f, -9.81f * m_gravityModifier, 0.0f);
    for (std::size_t i = 0; i < m_particles.size();)
    {
        Particle &particle = m_particles[i];
        particle.age += deltaTime;
        if (particle.age >= particle.lifetime)
        {
            m_particles[i] = m_particles.back();
            m_particles.pop_back();
            continue;
        }

        const float normalizedLife = std::clamp(particle.age / particle.lifetime, 0.0f, 1.0f);
        const float velocityMultiplier = EvaluateCurve(m_velocityOverLifetime, normalizedLife);
        particle.velocity += gravity * deltaTime;
        particle.position += particle.velocity * velocityMultiplier * deltaTime;
        particle.size = particle.baseSize * EvaluateCurve(m_sizeOverLifetime, normalizedLife);
        particle.rotation += particle.angularVelocity * deltaTime + EvaluateCurve(m_rotationOverLifetime, normalizedLife) * deltaTime;
        ++i;
    }
}

void ParticleSystemComponent::ResetBurstTriggers()
{
    for (Burst &burst : m_bursts)
        burst.triggered = false;
}

void ParticleSystemComponent::UpdateBurstEmission()
{
    for (Burst &burst : m_bursts)
    {
        if (!burst.triggered && m_systemTime >= burst.time)
        {
            Emit(burst.count);
            burst.triggered = true;
        }
    }
}

void ParticleSystemComponent::UpdateImpl()
{
    const float deltaTime = Time::Get().GetDeltaTime();
    UpdateParticles(deltaTime);
    if (m_debugDraw)
        DrawDebugParticles();
    if (!m_playing)
        return;

    if (m_delayElapsed < m_startDelay)
    {
        m_delayElapsed += deltaTime;
        return;
    }

    m_systemTime += deltaTime;
    if (m_systemTime >= m_duration)
    {
        if (m_looping)
        {
            m_systemTime = std::fmod(m_systemTime, m_duration);
            m_emissionAccumulator = 0.0f;
            ResetBurstTriggers();
        }
        else
        {
            m_systemTime = m_duration;
            m_playing = false;
        }
    }

    UpdateBurstEmission();
    m_emissionAccumulator += m_emissionRate * deltaTime;
    uint32_t toEmit = static_cast<uint32_t>(m_emissionAccumulator);
    m_emissionAccumulator -= static_cast<float>(toEmit);
    Emit(toEmit);
}

void ParticleSystemComponent::EditorUpdateImpl()
{
    const float deltaTime = Time::Get().GetDeltaTime();
    UpdateParticles(deltaTime);
    if (m_debugDraw)
        DrawDebugParticles();
}

void ParticleSystemComponent::DrawDebugParticles() const
{
    using Droplet::Graphics::Debug::DebugDraw;

    for (const Particle &particle : m_particles)
    {
        const float radius = std::max(0.001f, particle.size * 0.5f);
        DebugDraw::DrawSphere(
            Droplet::Math::Sphere(particle.position, radius),
            particle.color,
            1,
            false);
    }
}

void ParticleSystemComponent::QueueRenderImpl()
{
    if (GetOwner() && GetOwner()->GetScene())
        GetOwner()->GetScene()->QueueRenderable(shared_from_this());
}

RenderData ParticleSystemComponent::GetRenderDataImpl()
{
    RenderData renderData{};
    renderData.texData.GUID = m_texture.GetGUID();
    renderData.transparent = m_transparent;
    return renderData;
}

void ParticleSystemComponent::SerializeCurve(nlohmann::json &json, const char *key, const ScalarCurve &curve) const
{
    nlohmann::json curveJson;
    curveJson["type"] = static_cast<int>(curve.type);
    curveJson["points"] = nlohmann::json::array();
    for (const auto &point : curve.points)
    {
        curveJson["points"].push_back({
            {"position", { point.position.x, point.position.y }},
            {"controlPoint1", { point.controlPoint1.x, point.controlPoint1.y }},
            {"controlPoint2", { point.controlPoint2.x, point.controlPoint2.y }}
        });
    }
    json[key] = curveJson;
}

void ParticleSystemComponent::DeserializeCurve(const nlohmann::json &json, const char *key, ScalarCurve &curve)
{
    if (!json.contains(key))
        return;

    const nlohmann::json &curveJson = json[key];
    if (curveJson.contains("type"))
        curve.type = static_cast<Droplet::Math::Curves::CurveType>(curveJson["type"].get<int>());

    if (!curveJson.contains("points") || !curveJson["points"].is_array())
        return;

    curve.points.clear();
    for (const nlohmann::json &pointJson : curveJson["points"])
    {
        if (!pointJson.contains("position"))
            continue;
        Droplet::Math::Curves::Point<glm::vec2> point;
        point.position = glm::vec2(pointJson["position"][0].get<float>(), pointJson["position"][1].get<float>());
        if (pointJson.contains("controlPoint1"))
            point.controlPoint1 = glm::vec2(pointJson["controlPoint1"][0].get<float>(), pointJson["controlPoint1"][1].get<float>());
        if (pointJson.contains("controlPoint2"))
            point.controlPoint2 = glm::vec2(pointJson["controlPoint2"][0].get<float>(), pointJson["controlPoint2"][1].get<float>());
        curve.points.push_back(point);
    }
    if (curve.points.empty())
        curve = MakeDefaultCurve(1.0f);
}

nlohmann::json ParticleSystemComponent::SerializeImpl()
{
    nlohmann::json json;
    json["duration"] = m_duration;
    json["looping"] = m_looping;
    json["playOnAwake"] = m_playOnAwake;
    json["startDelay"] = m_startDelay;
    json["maxParticles"] = m_maxParticles;
    json["emissionRate"] = m_emissionRate;
    json["gravityModifier"] = m_gravityModifier;
    json["emissionShape"] = static_cast<int>(m_emissionShape);
    json["simulationSpace"] = static_cast<int>(m_simulationSpace);
    json["shapeRadius"] = m_shapeRadius;
    json["shapeBoxSize"] = { m_shapeBoxSize.x, m_shapeBoxSize.y, m_shapeBoxSize.z };
    json["coneAngle"] = m_coneAngle;
    json["transparent"] = m_transparent;
    json["debugDraw"] = m_debugDraw;
    json["textureGUID"] = GetTextureGUID();
    json["textureDisplayName"] = m_textureDisplayName;

    json["lifetimeRange"] = { m_lifetimeRange.min, m_lifetimeRange.max };
    json["speedRange"] = { m_speedRange.min, m_speedRange.max };
    json["sizeRange"] = { m_sizeRange.min, m_sizeRange.max };
    json["rotationRange"] = { m_rotationRange.min, m_rotationRange.max };
    json["angularVelocityRange"] = { m_angularVelocityRange.min, m_angularVelocityRange.max };
    json["startColorRange"] = {
        { m_startColorRange.min.r, m_startColorRange.min.g, m_startColorRange.min.b, m_startColorRange.min.a },
        { m_startColorRange.max.r, m_startColorRange.max.g, m_startColorRange.max.b, m_startColorRange.max.a }
    };

    json["bursts"] = nlohmann::json::array();
    for (const Burst &burst : m_bursts)
        json["bursts"].push_back({ { "time", burst.time }, { "count", burst.count } });

    SerializeCurve(json, "sizeOverLifetime", m_sizeOverLifetime);
    SerializeCurve(json, "velocityOverLifetime", m_velocityOverLifetime);
    SerializeCurve(json, "rotationOverLifetime", m_rotationOverLifetime);
    return json;
}

void ParticleSystemComponent::DeserializeImpl(const nlohmann::json p_compJson)
{
    if (p_compJson.contains("duration")) SetDuration(p_compJson["duration"].get<float>());
    if (p_compJson.contains("looping")) SetLooping(p_compJson["looping"].get<bool>());
    if (p_compJson.contains("playOnAwake")) SetPlayOnAwake(p_compJson["playOnAwake"].get<bool>());
    if (p_compJson.contains("startDelay")) SetStartDelay(p_compJson["startDelay"].get<float>());
    if (p_compJson.contains("maxParticles")) SetMaxParticles(p_compJson["maxParticles"].get<uint32_t>());
    if (p_compJson.contains("emissionRate")) SetEmissionRate(p_compJson["emissionRate"].get<float>());
    if (p_compJson.contains("gravityModifier")) SetGravityModifier(p_compJson["gravityModifier"].get<float>());
    if (p_compJson.contains("emissionShape"))
        SetEmissionShape(static_cast<EmissionShape>(p_compJson["emissionShape"].get<int>()));
    if (p_compJson.contains("simulationSpace"))
        SetSimulationSpace(static_cast<SimulationSpace>(p_compJson["simulationSpace"].get<int>()));
    if (p_compJson.contains("shapeRadius")) SetShapeRadius(p_compJson["shapeRadius"].get<float>());
    if (p_compJson.contains("shapeBoxSize"))
    {
        const auto &v = p_compJson["shapeBoxSize"];
        SetShapeBoxSize(glm::vec3(v[0].get<float>(), v[1].get<float>(), v[2].get<float>()));
    }
    if (p_compJson.contains("coneAngle")) SetConeAngle(p_compJson["coneAngle"].get<float>());
    if (p_compJson.contains("transparent")) m_transparent = p_compJson["transparent"].get<bool>();
    if (p_compJson.contains("debugDraw")) SetDebugDraw(p_compJson["debugDraw"].get<bool>());
    if (p_compJson.contains("textureGUID")) SetTextureGUID(p_compJson["textureGUID"].get<GUID>());
    if (p_compJson.contains("textureDisplayName")) m_textureDisplayName = p_compJson["textureDisplayName"].get<std::string>();

    if (p_compJson.contains("lifetimeRange"))
    {
        const auto &v = p_compJson["lifetimeRange"];
        SetLifetimeRange({ v[0].get<float>(), v[1].get<float>() });
    }
    if (p_compJson.contains("speedRange"))
    {
        const auto &v = p_compJson["speedRange"];
        SetSpeedRange({ v[0].get<float>(), v[1].get<float>() });
    }
    if (p_compJson.contains("sizeRange"))
    {
        const auto &v = p_compJson["sizeRange"];
        SetSizeRange({ v[0].get<float>(), v[1].get<float>() });
    }
    if (p_compJson.contains("rotationRange"))
    {
        const auto &v = p_compJson["rotationRange"];
        SetRotationRange({ v[0].get<float>(), v[1].get<float>() });
    }
    if (p_compJson.contains("angularVelocityRange"))
    {
        const auto &v = p_compJson["angularVelocityRange"];
        SetAngularVelocityRange({ v[0].get<float>(), v[1].get<float>() });
    }
    if (p_compJson.contains("startColorRange"))
    {
        const auto &v = p_compJson["startColorRange"];
        m_startColorRange = {
            glm::vec4(v[0][0].get<float>(), v[0][1].get<float>(), v[0][2].get<float>(), v[0][3].get<float>()),
            glm::vec4(v[1][0].get<float>(), v[1][1].get<float>(), v[1][2].get<float>(), v[1][3].get<float>())
        };
    }

    ClearBursts();
    if (p_compJson.contains("bursts"))
        for (const nlohmann::json &burst : p_compJson["bursts"])
            AddBurst(burst["time"].get<float>(), burst["count"].get<uint32_t>());

    DeserializeCurve(p_compJson, "sizeOverLifetime", m_sizeOverLifetime);
    DeserializeCurve(p_compJson, "velocityOverLifetime", m_velocityOverLifetime);
    DeserializeCurve(p_compJson, "rotationOverLifetime", m_rotationOverLifetime);
}

void ParticleSystemComponent::RenderUIImpl()
{
#ifdef _EDITOR
    using namespace ImGui;

    PushID(this);

    if (Button(m_playing ? "Pause" : "Play"))
    {
        if (m_playing)
            Pause();
        else
            Play();
    }
    SameLine();
    if (Button("Stop"))
        Stop(true);
    SameLine();
    if (Button("Burst"))
        Emit(std::max(1u, m_bursts.empty() ? 30u : m_bursts.front().count));

    Checkbox("Looping", &m_looping);
    Checkbox("Play On Awake", &m_playOnAwake);
    Checkbox("Transparent", &m_transparent);
    Checkbox("Debug Draw", &m_debugDraw);
    DragFloat("Duration", &m_duration, 0.1f, 0.001f, 3600.0f);
    DragFloat("Start Delay", &m_startDelay, 0.05f, 0.0f, 3600.0f);
    DragFloat("Emission Rate", &m_emissionRate, 0.5f, 0.0f, 100000.0f);

    int maxParticles = static_cast<int>(m_maxParticles);
    if (DragInt("Max Particles", &maxParticles, 1.0f, 1, 1000000))
        SetMaxParticles(static_cast<uint32_t>(maxParticles));

    const char *shapeNames[] = { "Point", "Sphere", "Hemisphere", "Box", "Cone", "Circle" };
    int shape = static_cast<int>(m_emissionShape);
    if (Combo("Emission Shape", &shape, shapeNames, IM_ARRAYSIZE(shapeNames)))
        m_emissionShape = static_cast<EmissionShape>(shape);

    const char *spaceNames[] = { "Local", "World" };
    int space = static_cast<int>(m_simulationSpace);
    if (Combo("Simulation Space", &space, spaceNames, IM_ARRAYSIZE(spaceNames)))
        m_simulationSpace = static_cast<SimulationSpace>(space);

    DragFloat("Shape Radius", &m_shapeRadius, 0.05f, 0.0f, 1000.0f);
    DragFloat3("Box Size", &m_shapeBoxSize.x, 0.05f, 0.0f, 1000.0f);
    DragFloat("Cone Angle", &m_coneAngle, 0.5f, 0.0f, 180.0f);
    DragFloat("Gravity Modifier", &m_gravityModifier, 0.05f, -10.0f, 10.0f);

    DragFloat2("Lifetime Range", &m_lifetimeRange.min, 0.05f, 0.001f, 3600.0f);
    DragFloat2("Speed Range", &m_speedRange.min, 0.05f, -1000.0f, 1000.0f);
    DragFloat2("Size Range", &m_sizeRange.min, 0.01f, 0.0f, 1000.0f);
    DragFloat2("Rotation Range", &m_rotationRange.min, 1.0f, -3600.0f, 3600.0f);
    DragFloat2("Angular Velocity", &m_angularVelocityRange.min, 1.0f, -3600.0f, 3600.0f);
    ColorEdit4("Start Color Min", &m_startColorRange.min.r);
    ColorEdit4("Start Color Max", &m_startColorRange.max.r);

    Separator();
    TextUnformatted("Bursts");
    for (std::size_t i = 0; i < m_bursts.size();)
    {
        PushID(static_cast<int>(i));
        float time = m_bursts[i].time;
        int count = static_cast<int>(m_bursts[i].count);
        DragFloat("Time", &time, 0.05f, 0.0f, m_duration);
        DragInt("Count", &count, 1.0f, 1, 1000000);
        m_bursts[i].time = std::max(0.0f, time);
        m_bursts[i].count = static_cast<uint32_t>(std::max(1, count));
        SameLine();
        if (Button("Remove"))
            m_bursts.erase(m_bursts.begin() + i);
        else
            ++i;
        PopID();
    }
    if (Button("Add Burst"))
        AddBurst(0.0f, 30);

    Separator();
    GUID newGUID = C_INVALID_GUID;
    std::string newName;
    if (BeginTable("ParticleTexture", 2, ImGuiTableFlags_SizingStretchProp))
    {
        TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);
        if (ResourceSlot("Texture", m_texture.GetGUID(), m_textureDisplayName, newGUID, newName, StaticResourceType::Texture2D))
            SetTextureGUID(newGUID);
        EndTable();
    }

    auto RenderCurve = [](const char *label, ScalarCurve &curve)
    {
        std::vector<ImGui::BezierPoint> points;
        points.reserve(curve.points.size());
        for (const auto &point : curve.points)
        {
            points.push_back({
                ImVec2(point.position.x, point.position.y),
                ImVec2(point.controlPoint1.x, point.controlPoint1.y),
                ImVec2(point.controlPoint2.x, point.controlPoint2.y)
            });
        }
        ImGuiCurveEditFlags flags = ImGuiCurveEditFlags_ClampPoints | ImGuiCurveEditFlags_ForceSpanWidth | ImGuiCurveEditFlags_ForceInjective;
        if (ImGui::CurveEdit(label, &points, ImVec2(0, 150), ImRect(0.0f, 0.0f, 1.0f, 2.0f), 2.0f,
            ImRect(48, 24, 20, 24), ImVec2i(4, 4), flags))
        {
            curve.points.clear();
            for (const ImGui::BezierPoint &point : points)
            {
                Droplet::Math::Curves::Point<glm::vec2> value;
                value.position = glm::vec2(point.position.x, point.position.y);
                value.controlPoint1 = glm::vec2(point.controlPoint1.x, point.controlPoint1.y);
                value.controlPoint2 = glm::vec2(point.controlPoint2.x, point.controlPoint2.y);
                curve.points.push_back(value);
            }
        }
    };

    if (CollapsingHeader("Over Lifetime", ImGuiTreeNodeFlags_DefaultOpen))
    {
        RenderCurve("Size Over Lifetime", m_sizeOverLifetime);
        RenderCurve("Velocity Over Lifetime", m_velocityOverLifetime);
        RenderCurve("Rotation Over Lifetime", m_rotationOverLifetime);
    }

    PopID();
#endif
}


void ParticleSystemComponent::GenerateParticleMesh(std::vector<ParticleVertex> &outVertices) const
{
    // TODO
}
