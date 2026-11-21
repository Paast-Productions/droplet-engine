#include "ParticleEmitterComponent.hpp"

#include <SceneSystem/Node.hpp>
#include <SceneSystem/Scene.hpp>
#include <Time.hpp>
#include <resource/ResourceManager.hpp>
#include <Editor/ResourceSlot.hpp>
#include <Editor/ImGuiCurve.hpp>
#include <Graphics/DebugDraw.hpp>
#include <Math/Bounds/Sphere.hpp>
#include <Math/Bounds/AABB.hpp>
#include <Math/Bounds/OBB.hpp>
#include <cmath>
#include <algorithm>
#include <cstddef>

using namespace Droplet;
using namespace Droplet::Math;
using namespace Droplet::Scene;
using namespace Droplet::Graphics;

ParticleEmitterComponent::ParticleEmitterComponent()
{
	m_particles.reserve(m_maxParticles);
}

ParticleEmitterComponent::ScalarCurve ParticleEmitterComponent::MakeDefaultCurve(float value)
{
	ScalarCurve curve;
	curve.type = Math::Curves::CurveType::Linear;
	curve.points = {
		{ glm::vec2(0.0f, value), glm::vec2(0.0f, value), glm::vec2(0.0f, value) },
		{ glm::vec2(1.0f, value), glm::vec2(1.0f, value), glm::vec2(1.0f, value) }
	};
	return curve;
}

void ParticleEmitterComponent::StartImpl()
{
	SetRenderable();
	if (m_playOnAwake)
		Play();
}

void ParticleEmitterComponent::Play()
{
	m_paused = false;
	m_playing = true;
}

void ParticleEmitterComponent::Pause()
{
	m_paused = true;
	m_playing = false;
}

void ParticleEmitterComponent::Stop(bool clearParticles)
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

void ParticleEmitterComponent::SetPlaying(bool playing)
{
	if (playing)
		Play();
	else
		Pause();
}

void ParticleEmitterComponent::Emit(uint32_t count)
{
	if (m_particles.capacity() < m_maxParticles)
		m_particles.reserve(m_maxParticles);
	const uint32_t available = m_maxParticles - static_cast<uint32_t>(m_particles.size());
	for (uint32_t i = 0; i < count && i < available; ++i)
		SpawnParticle();
}

void ParticleEmitterComponent::SetDuration(float duration) { m_duration = std::max(0.001f, duration); }
void ParticleEmitterComponent::SetLooping(bool looping) { m_looping = looping; }
void ParticleEmitterComponent::SetPlayOnAwake(bool playOnAwake) { m_playOnAwake = playOnAwake; }
void ParticleEmitterComponent::SetStartDelay(float startDelay) { m_startDelay = std::max(0.0f, startDelay); }
void ParticleEmitterComponent::SetMaxParticles(uint32_t maxParticles)
{
	m_maxParticles = std::max(1u, maxParticles);
	m_particles.reserve(m_maxParticles);
	if (m_particles.size() > m_maxParticles)
		m_particles.resize(m_maxParticles);
}
void ParticleEmitterComponent::SetEmissionRate(float particlesPerSecond) { m_emissionRate = std::max(0.0f, particlesPerSecond); }
void ParticleEmitterComponent::SetLifetimeRange(const FloatRange &range) { m_lifetimeRange = { std::max(0.001f, range.min), std::max(0.001f, range.max) }; }
void ParticleEmitterComponent::SetSpeedRange(const FloatRange &range) { m_speedRange = range; }
void ParticleEmitterComponent::SetSizeRange(const FloatRange &range) { m_sizeRange = { std::max(0.0f, range.min), std::max(0.0f, range.max) }; }
void ParticleEmitterComponent::SetRotationRange(const FloatRange &range) { m_rotationRange = range; }
void ParticleEmitterComponent::SetAngularVelocityRange(const FloatRange &range) { m_angularVelocityRange = range; }
void ParticleEmitterComponent::SetStartColorRange(const ColorRange &range) { m_startColorRange = range; }
void ParticleEmitterComponent::SetStartColorMode(StartColorMode mode) { m_startColorMode = mode; }
void ParticleEmitterComponent::SetGravityModifier(float gravityModifier) { m_gravityModifier = gravityModifier; }
void ParticleEmitterComponent::SetEmissionShape(EmissionShape shape) { m_emissionShape = shape; }
void ParticleEmitterComponent::SetSimulationSpace(SimulationSpace space) { m_simulationSpace = space; }
void ParticleEmitterComponent::SetShapeRadius(float radius) { m_shapeRadius = std::max(0.0f, radius); }
void ParticleEmitterComponent::SetShapeBoxSize(const glm::vec3 &size) { m_shapeBoxSize = glm::max(size, glm::vec3(0.0f)); }
void ParticleEmitterComponent::SetConeAngle(float angleDegrees) { m_coneAngle = std::clamp(angleDegrees, 0.0f, 180.0f); }
void ParticleEmitterComponent::AddBurst(float time, uint32_t count)
{
	m_bursts.push_back({ std::max(0.0f, time), std::max(1u, count), false });
	std::sort(m_bursts.begin(), m_bursts.end(), [](const Burst &a, const Burst &b) { return a.time < b.time; });
}
void ParticleEmitterComponent::ClearBursts()
{
	m_bursts.clear();
}

void ParticleEmitterComponent::SetTextureGUID(GUID guid)
{
	m_texture = ResourceManager::GetInstance().LoadResource<Texture2DResource>(guid);
	if (MetaEntry entry; ResourceManager::GetInstance().GetResourceMetaEntry(guid, entry))
		m_textureDisplayName = entry.displayName;
}

float ParticleEmitterComponent::RandomFloat(float min, float max)
{
	if (max < min)
		std::swap(min, max);
	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(m_random);
}

glm::vec3 ParticleEmitterComponent::RandomUnitSphere()
{
	const float z = RandomFloat(-1.0f, 1.0f);
	const float angle = RandomFloat(0.0f, glm::two_pi<float>());
	const float radius = std::sqrt(std::max(0.0f, 1.0f - z * z));
	return glm::vec3(radius * std::cos(angle), radius * std::sin(angle), z);
}

glm::vec4 ParticleEmitterComponent::RandomStartColor()
{
	switch (m_startColorMode)
	{
	case StartColorMode::Lerp:
		return glm::mix(m_startColorRange.min, m_startColorRange.max, RandomFloat(0.0f, 1.0f));
	case StartColorMode::SphericalLerp:
	{
		glm::vec4 start = m_startColorRange.min;
		glm::vec4 end = m_startColorRange.max;
		const float startLength = glm::length(start);
		const float endLength = glm::length(end);
		if (startLength <= 0.0001f || endLength <= 0.0001f)
			return glm::mix(start, end, RandomFloat(0.0f, 1.0f));

		start /= startLength;
		end /= endLength;
		float dot = std::clamp(glm::dot(start, end), -1.0f, 1.0f);
		const float t = RandomFloat(0.0f, 1.0f);
		if (dot > 0.9995f)
			return glm::normalize(glm::mix(start, end, t));
		if (dot < -0.9995f)
			return glm::mix(m_startColorRange.min, m_startColorRange.max, t);

		const float theta = std::acos(dot);
		const float sinTheta = std::sin(theta);
		const glm::vec4 result =
			(std::sin((1.0f - t) * theta) / sinTheta) * start +
			(std::sin(t * theta) / sinTheta) * end;
		return glm::clamp(glm::normalize(result), glm::vec4(0.0f), glm::vec4(1.0f));
	}
	case StartColorMode::RandomPerComponent:
	default:
		return glm::vec4(
			RandomFloat(m_startColorRange.min.r, m_startColorRange.max.r),
			RandomFloat(m_startColorRange.min.g, m_startColorRange.max.g),
			RandomFloat(m_startColorRange.min.b, m_startColorRange.max.b),
			RandomFloat(m_startColorRange.min.a, m_startColorRange.max.a));
	}
}

glm::vec3 ParticleEmitterComponent::RandomDirectionForEmission()
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

void ParticleEmitterComponent::SpawnParticle()
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
		localPosition.z = std::abs(localPosition.z);
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

	glm::vec3 worldDirection = glm::normalize(direction);
	glm::vec3 worldPosition = localPosition;
	if (GetOwner())
	{
		Transform &transform = GetOwner()->GetTransform();
		worldPosition = m_simulationSpace == SimulationSpace::Local
			? glm::vec3(transform.GetMatrix(Transform::Space::World) * glm::vec4(localPosition, 1.0f))
			: transform.GetPosition(Transform::Space::World) + glm::mat3(transform.GetRotation(Transform::Space::World)) * localPosition;
		worldDirection = glm::normalize(glm::mat3(transform.GetRotation(Transform::Space::World)) * direction);
	}

	const float lifetime = RandomFloat(m_lifetimeRange.min, m_lifetimeRange.max);
	const float speed = RandomFloat(m_speedRange.min, m_speedRange.max);
	const glm::vec4 startColor = RandomStartColor();

	Particle particle;
	particle.position = worldPosition;
	particle.velocity = worldDirection * speed;
	particle.baseSize = RandomFloat(m_sizeRange.min, m_sizeRange.max);
	particle.size = particle.baseSize;
	particle.rotation = glm::radians(RandomFloat(m_rotationRange.min, m_rotationRange.max));
	particle.angularVelocity = glm::radians(RandomFloat(m_angularVelocityRange.min, m_angularVelocityRange.max));
	particle.color = startColor;
	particle.lifetime = lifetime;
	m_particles.push_back(particle);
}

float ParticleEmitterComponent::EvaluateCurve(const ScalarCurve &curve, float normalizedTime) const
{
	if (curve.points.empty())
		return 0.0f;
	if (curve.points.size() == 1)
		return curve.points.front().position.y;
	normalizedTime = std::clamp(normalizedTime, 0.0f, 1.0f);
	if (curve.IsInjective())
		return curve.SampleInjective(normalizedTime);

	return curve.SamplePoint(normalizedTime).y;
}

void ParticleEmitterComponent::UpdateEmitter(float deltaTime)
{
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

void ParticleEmitterComponent::UpdateParticles(float deltaTime)
{
	const glm::vec3 gravity(0.0f, -9.81f * m_gravityModifier, 0.0f);
	#pragma omp parallel for schedule(static)
	for (std::ptrdiff_t i = 0; i < static_cast<std::ptrdiff_t>(m_particles.size()); ++i)
	{
		Particle &particle = m_particles[static_cast<std::size_t>(i)];
		particle.age += deltaTime;
		if (particle.age < particle.lifetime)
		{
			const float normalizedLife = std::clamp(particle.age / particle.lifetime, 0.0f, 1.0f);
			const float velocityMultiplier = EvaluateCurve(m_velocityOverLifetime, normalizedLife);
			particle.velocity += gravity * deltaTime;
			particle.position += particle.velocity * velocityMultiplier * deltaTime;
			particle.size = particle.baseSize * EvaluateCurve(m_sizeOverLifetime, normalizedLife);
			particle.rotation += particle.angularVelocity * deltaTime + EvaluateCurve(m_rotationOverLifetime, normalizedLife) * deltaTime;
		}
	}

	std::size_t writeIndex = 0;
	for (std::size_t readIndex = 0; readIndex < m_particles.size(); ++readIndex)
	{
		if (m_particles[readIndex].age < m_particles[readIndex].lifetime)
		{
			if (writeIndex != readIndex)
				m_particles[writeIndex] = m_particles[readIndex];
			++writeIndex;
		}
	}
	m_particles.resize(writeIndex);
}

void ParticleEmitterComponent::ResetBurstTriggers()
{
	for (Burst &burst : m_bursts)
		burst.triggered = false;
}

void ParticleEmitterComponent::UpdateBurstEmission()
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

void ParticleEmitterComponent::UpdateImpl()
{
	if (EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::InEditor))
		return;

	const float deltaTime = Time::Get().GetDeltaTime();
	UpdateEmitter(deltaTime);
}

void ParticleEmitterComponent::EditorUpdateImpl()
{
	if (!EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::InEditor))
		return;

	const float deltaTime = Time::Get().GetDeltaTime();
	UpdateEmitter(deltaTime);

	if (m_uiSelected)
	{
		DrawDebugEmitter();
	}
	m_uiSelected = false;
}

void ParticleEmitterComponent::DrawDebugParticles() const
{
	ZoneScoped;

	using Droplet::Graphics::Debug::DebugDraw;

	for (const Particle &particle : m_particles)
	{
		const float radius = std::max(0.001f, particle.size * 0.5f);
		DebugDraw::DrawSphere(
			Sphere(particle.position, radius),
			particle.color,
			1,
			false);
	}
}

void ParticleEmitterComponent::DrawDebugEmitter() const
{
	ZoneScoped;

	using Droplet::Graphics::Debug::DebugDraw;

	glm::vec3 position(0.0f);
	glm::vec3 forward(0.0f, 0.0f, 1.0f);
	glm::vec3 up(0.0f, 1.0f, 0.0f);
	glm::vec3 right(1.0f, 0.0f, 0.0f);
	float radiusScale = 1.0f;

	if (GetOwner())
	{
		Transform &transform = GetOwner()->GetTransform();
		position = transform.GetPosition(Transform::Space::World);
		forward = transform.GetForward(Transform::Space::World);
		up = transform.GetUp(Transform::Space::World);
		right = transform.GetRight(Transform::Space::World);
		const glm::mat4 worldMatrix = transform.GetMatrix(Transform::Space::World);
		radiusScale = std::max({
			glm::length(glm::vec3(worldMatrix[0])),
			glm::length(glm::vec3(worldMatrix[1])),
			glm::length(glm::vec3(worldMatrix[2]))
		});
	}

	const glm::vec4 color(0.25f, 0.75f, 1.0f, 0.75f);
	const float radius = std::max(0.001f, m_shapeRadius * radiusScale);

	switch (m_emissionShape)
	{
	case EmissionShape::Point:
		DebugDraw::DrawSphere(Sphere(position, 0.05f * radiusScale), color, 1, false);
		break;

	case EmissionShape::Sphere:
		DebugDraw::DrawWireSphere(Sphere(position, radius), color, 24, false);
		break;

	case EmissionShape::Hemisphere:
	{
		DebugDraw::DrawCircle(position, forward, radius, color, 24, false, true);
		DebugDraw::DrawArc(position, -up, right * radius, radius, 180.0f, color, 12, false, true);
		DebugDraw::DrawArc(position, right, up * radius, radius, 180.0f, color, 12, false, true);
		break;
	}

	case EmissionShape::Box:
	{
		if (GetOwner())
		{
			Transform &transform = GetOwner()->GetTransform();
			OBB obb(
				position,
				m_shapeBoxSize * 0.5f,
				glm::mat3(transform.GetRotation(Transform::Space::World)));
			DebugDraw::DrawBox(obb, color, false, true);
		}
		else
		{
			DebugDraw::DrawBox(
				AABB(position, m_shapeBoxSize * 0.5f),
				color,
				false,
				true);
		}
		break;
	}

	case EmissionShape::Cone:
	{
		const float angle = glm::radians(m_coneAngle);
		const float length = radius;
		const float baseRadius = length * std::tan(std::clamp(angle, 0.0f, glm::pi<float>() * 0.49f));
		const glm::vec3 baseCenter = position + forward * length;
		DebugDraw::DrawRay(position, forward * length, color, false);
		DebugDraw::DrawCircle(baseCenter, forward, baseRadius, color, 24, false, true);
		DebugDraw::DrawLine(position, baseCenter + right * baseRadius, color, false);
		DebugDraw::DrawLine(position, baseCenter - right * baseRadius, color, false);
		DebugDraw::DrawLine(position, baseCenter + up * baseRadius, color, false);
		DebugDraw::DrawLine(position, baseCenter - up * baseRadius, color, false);
		break;
	}

	case EmissionShape::Circle:
		DebugDraw::DrawCircle(position, forward, radius, color, 24, false, true);
		break;
	}
}

void ParticleEmitterComponent::QueueRenderImpl()
{
	if (GetOwner() && GetOwner()->GetScene())
		GetOwner()->GetScene()->QueueRenderable(shared_from_this());
}

RenderData ParticleEmitterComponent::GetRenderDataImpl()
{
	RenderData renderData{};
	renderData.texData.GUID = m_texture.GetGUID();
	renderData.transparent = m_transparent;
	return renderData;
}

void ParticleEmitterComponent::SerializeCurve(nlohmann::json &json, const char *key, const ScalarCurve &curve) const
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

void ParticleEmitterComponent::DeserializeCurve(const nlohmann::json &json, const char *key, ScalarCurve &curve)
{
	if (!json.contains(key))
		return;

	const nlohmann::json &curveJson = json[key];
	if (curveJson.contains("type"))
		curve.type = static_cast<Curves::CurveType>(curveJson["type"].get<int>());

	if (!curveJson.contains("points") || !curveJson["points"].is_array())
		return;

	curve.points.clear();
	for (const nlohmann::json &pointJson : curveJson["points"])
	{
		if (!pointJson.contains("position"))
			continue;
		Curves::Point<glm::vec2> point;
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

nlohmann::json ParticleEmitterComponent::SerializeImpl()
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
	json["startColorMode"] = static_cast<int>(m_startColorMode);
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

void ParticleEmitterComponent::DeserializeImpl(const nlohmann::json p_compJson)
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
	if (p_compJson.contains("startColorMode"))
		SetStartColorMode(static_cast<StartColorMode>(p_compJson["startColorMode"].get<int>()));
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

void ParticleEmitterComponent::RenderUIImpl()
{
#ifdef _EDITOR
	using namespace ImGui;

	m_uiSelected = true;

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
	const char *startColorModeNames[] = { "Random Per Component", "Lerp", "Spherical Lerp" };
	int startColorMode = static_cast<int>(m_startColorMode);
	if (Combo("Start Color Mode", &startColorMode, startColorModeNames, IM_ARRAYSIZE(startColorModeNames)))
		m_startColorMode = static_cast<StartColorMode>(startColorMode);

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

	auto RenderCurve = [](const char *label, ScalarCurve &curve) {
		BeginChild(label, ImVec2(ImGui::GetContentRegionAvail().x, 0), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);

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

		ImGuiStorage *storage = ImGui::GetStateStorage();

		const char *curveTypeNames[] = { "Linear", "Quadratic Bezier", "Cubic Bezier" };
		int curveType = static_cast<int>(curve.type);

		ImGuiID heightID = ImGui::GetID("height");
		ImGuiID minValueID = ImGui::GetID("minValue");
		ImGuiID maxValueID = ImGui::GetID("maxValue");
		ImGuiID lineThicknessID = ImGui::GetID("lineThickness");

		int height = storage->GetInt(heightID, 150);
		float minValue = storage->GetFloat(minValueID, 0.0f);
		float maxValue = storage->GetFloat(maxValueID, 2.0f);
		float lineThickness = storage->GetFloat(lineThicknessID, 3.0f);

		ImGuiCurveEditFlags flags = ImGuiCurveEditFlags_ClampPoints | ImGuiCurveEditFlags_ForceSpanWidth | ImGuiCurveEditFlags_ForceInjective;
		if (curve.type == Curves::CurveType::BezierQuadratic)    flags |= ImGuiCurveEditFlags_Quadratic;
		else if (curve.type == Curves::CurveType::Linear)        flags |= ImGuiCurveEditFlags_Linear;

		if (ImGui::CurveEdit(label, &points, ImVec2(0, height), ImRect(0.0f, minValue, 1.0f, maxValue), lineThickness,
			ImRect(48, 24, 20, 24), ImVec2i(4, 4), flags))
		{
			curve.points.clear();
			for (const ImGui::BezierPoint &point : points)
			{
				Curves::Point<glm::vec2> value;
				value.position = glm::vec2(point.position.x, point.position.y);
				value.controlPoint1 = glm::vec2(point.controlPoint1.x, point.controlPoint1.y);
				value.controlPoint2 = glm::vec2(point.controlPoint2.x, point.controlPoint2.y);
				curve.points.push_back(value);
			}
		}

		if (ImGui::BeginPopupContextItem("Curve Options"))
		{
			if (ImGui::Button("Reset"))
				curve = MakeDefaultCurve(1.0f);

			if (ImGui::Combo("Curve Type", &curveType, curveTypeNames, IM_ARRAYSIZE(curveTypeNames)))
				curve.type = static_cast<Curves::CurveType>(curveType);

			if (ImGui::DragInt("Height", &height, 1.0f, 1))
				storage->SetInt(heightID, height);

			ImVec2 minMaxValue(minValue, maxValue);
			if (ImGui::DragFloat2("Value Range", &minMaxValue.x, 0.01f))
			{
				storage->SetFloat(minValueID, minMaxValue.x);
				storage->SetFloat(maxValueID, minMaxValue.y);
			}

			if (ImGui::DragFloat("Line Thickness", &lineThickness, 0.1f, 1))
				storage->SetFloat(lineThicknessID, lineThickness);

			ImGui::EndPopup();
		}

		EndChild();
	};

	if (CollapsingHeader("Over Lifetime", ImGuiTreeNodeFlags_DefaultOpen))
	{
		TextUnformatted("Size Over Lifetime");
		RenderCurve("##Size Over Lifetime", m_sizeOverLifetime);

		TextUnformatted("Velocity Over Lifetime");
		RenderCurve("##Velocity Over Lifetime", m_velocityOverLifetime);

		TextUnformatted("Rotation Over Lifetime");
		RenderCurve("##Rotation Over Lifetime", m_rotationOverLifetime);
	}

	PopID();
#endif
}


void ParticleEmitterComponent::GenerateParticleMesh(std::vector<ParticleVertex> &outVertices) const
{
	ZoneScoped;

	// TODO
}
