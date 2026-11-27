#include "ParticleEmitterComponent.hpp"

#include <SceneSystem/Node.hpp>
#include <SceneSystem/Scene.hpp>
#include <Time.hpp>
#include <resource/ResourceManager.hpp>
#include <SceneSystem/SceneResourceLoad.hpp>
#include <Editor/ResourceSlot.hpp>
#include <Editor/ImGuiCurve.hpp>
#include <Graphics/Debug/DebugDraw.hpp>
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
using namespace Droplet::Common;

ParticleEmitterComponent::ParticleEmitterComponent()
{
	m_particles.reserve(m_maxParticles);

	if (EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::EditorFlags))
	{
		// Set default resources

		ResourceManager &rsMan = ResourceManager::GetInstance();
		MetaEntry entry;

		// Texture
		if (rsMan.GetResourceMetaEntry("White", entry))
		{
			SetTextureGUID(entry.guid);
		}

		// Vertex Shader
		if (rsMan.GetResourceMetaEntry("Particle.VS", entry))
		{
			SetVSGUID(entry.guid);
		}

		// Geometry Shader
		if (rsMan.GetResourceMetaEntry("Particle.GS", entry))
		{
			SetGSGUID(entry.guid);
		}

		// Fragment Shader
		if (rsMan.GetResourceMetaEntry("Particle.FS", entry))
		{
			SetFSGUID(entry.guid);
		}
	}
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
	// Resuming from pause continues the sequence; a fresh play restarts it so the same seed gives the same result.
	// Also restarts a non-looping emitter that already expired, whose timers would otherwise end it immediately.
	if (!m_paused)
	{
		Stop(false);
		if (m_randomSeedEachPlay)
			SetRandomSeed(std::random_device{}() & 0x7FFFFFFFu);
		else
			m_random.seed(m_randomSeed);
		RollEmissionRate();
	}
	m_paused = false;
	m_playing = true;
	// Movement made while stopped or paused should not count as emitter travel
	m_hasLastOwnerPosition = false;
	m_emitterVelocity = glm::vec3(0.0f);
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
	m_updateAccumulator = 0.0f;
	m_turbulenceTime = 0.0f;
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
void ParticleEmitterComponent::SetTimeScale(float timeScale) { m_timeScale = std::max(0.0f, timeScale); }
void ParticleEmitterComponent::SetUpdateRate(float updatesPerSecond) { m_updateRate = std::max(0.0f, updatesPerSecond); }
void ParticleEmitterComponent::SetMaxParticles(uint32_t maxParticles)
{
	m_maxParticles = std::max(1u, maxParticles);
	m_particles.reserve(m_maxParticles);
	if (m_particles.size() > m_maxParticles)
		m_particles.resize(m_maxParticles);
}
void ParticleEmitterComponent::SetEmissionRate(float particlesPerSecond) { m_emissionRate = std::max(0.0f, particlesPerSecond); }
void ParticleEmitterComponent::SetEmissionRateOverDistance(float particlesPerUnit) { m_emissionRateOverDistance = std::max(0.0f, particlesPerUnit); }
void ParticleEmitterComponent::SetRandomizeEmissionRate(bool randomize)
{
	m_randomizeEmissionRate = randomize;
	RollEmissionRate();
}
void ParticleEmitterComponent::SetEmissionRateRange(const FloatRange &range)
{
	const float min = std::max(0.0f, range.min);
	m_emissionRateRange = { min, std::max(min, range.max) };
	RollEmissionRate();
}
void ParticleEmitterComponent::SetInheritVelocity(float inheritVelocity) { m_inheritVelocity = std::max(0.0f, inheritVelocity); }
void ParticleEmitterComponent::SetLifetimeRange(const FloatRange &range) { m_lifetimeRange = { std::max(0.001f, range.min), std::max(0.001f, range.max) }; }
void ParticleEmitterComponent::SetSpeedRange(const FloatRange &range) { m_speedRange = range; }
void ParticleEmitterComponent::SetWidthRange(const FloatRange &range) { m_widthRange = { std::max(0.0f, range.min), std::max(0.0f, range.max) }; }
void ParticleEmitterComponent::SetHeightRange(const FloatRange &range) { m_heightRange = { std::max(0.0f, range.min), std::max(0.0f, range.max) }; }
void ParticleEmitterComponent::SetRotationRange(const FloatRange &range) { m_rotationRange = range; }
void ParticleEmitterComponent::SetBillboardOffset(const glm::vec2 &offset) { m_billboardOffset = offset; }
void ParticleEmitterComponent::SetAngularVelocityRange(const FloatRange &range) { m_angularVelocityRange = range; }
void ParticleEmitterComponent::SetStartColorRange(const ColorRange &range) { m_startColorRange = range; }
void ParticleEmitterComponent::SetStartColorMode(StartColorMode mode) { m_startColorMode = mode; }
void ParticleEmitterComponent::SetGravityModifier(float gravityModifier) { m_gravityModifier = gravityModifier; }
void ParticleEmitterComponent::SetEmissionShape(EmissionShape shape) { m_emissionShape = shape; }
void ParticleEmitterComponent::SetSimulationSpace(SimulationSpace space) { m_simulationSpace = space; }
void ParticleEmitterComponent::SetShapeRadius(float radius) { m_shapeRadius = std::max(0.0f, radius); }
void ParticleEmitterComponent::SetShapeBoxSize(const glm::vec3 &size) { m_shapeBoxSize = glm::max(size, glm::vec3(0.0f)); }
void ParticleEmitterComponent::SetConeAngle(float angleDegrees) { m_coneAngle = std::clamp(angleDegrees, 0.0f, 180.0f); }
void ParticleEmitterComponent::SetRandomSeed(uint32_t seed)
{
	m_randomSeed = seed;
	m_random.seed(m_randomSeed);
}

void ParticleEmitterComponent::SetColorOverLifetime(std::vector<ColorStop> stops)
{
	m_colorOverLifetime = std::move(stops);
	if (m_colorOverLifetime.empty())
		m_colorOverLifetime = { { 0.0f, glm::vec4(1.0f) }, { 1.0f, glm::vec4(1.0f) } };
	SortColorOverLifetime();
}

void ParticleEmitterComponent::SortColorOverLifetime()
{
	for (ColorStop &stop : m_colorOverLifetime)
		stop.time = std::clamp(stop.time, 0.0f, 1.0f);
	std::stable_sort(m_colorOverLifetime.begin(), m_colorOverLifetime.end(),
		[](const ColorStop &a, const ColorStop &b) { return a.time < b.time; });
}

void ParticleEmitterComponent::AddBurst(float time, uint32_t count)
{
	m_bursts.push_back({ std::max(0.0f, time), std::max(1u, count), false });
	std::sort(m_bursts.begin(), m_bursts.end(), [](const Burst &a, const Burst &b) { return a.time < b.time; });
}
void ParticleEmitterComponent::ClearBursts()
{
	m_bursts.clear();
}

namespace
{
	glm::vec3 NormalizeOr(const glm::vec3 &p_v, const glm::vec3 &p_fallback)
	{
		return glm::length(p_v) > 1e-6f ? glm::normalize(p_v) : p_fallback;
	}

	/// @brief Any unit vector perpendicular to p_axis (which must be normalized)
	glm::vec3 AnyPerpendicular(const glm::vec3 &p_axis)
	{
		const glm::vec3 helper = std::abs(p_axis.y) < 0.999f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
		return glm::normalize(glm::cross(p_axis, helper));
	}

	/// @brief Linearly interpolates the color stops at p_t. Stops must be sorted by time.
	glm::vec4 EvaluateGradient(const std::vector<ParticleEmitterComponent::ColorStop> &p_stops, float p_t)
	{
		if (p_stops.empty())
			return glm::vec4(1.0f);
		if (p_t <= p_stops.front().time)
			return p_stops.front().color;

		for (std::size_t i = 1; i < p_stops.size(); ++i)
		{
			const ParticleEmitterComponent::ColorStop &b = p_stops[i];
			if (p_t < b.time)
			{
				const ParticleEmitterComponent::ColorStop &a = p_stops[i - 1];
				const float span = b.time - a.time;
				const float u = span > 0.0f ? (p_t - a.time) / span : 1.0f;
				return glm::mix(a.color, b.color, u);
			}
		}
		return p_stops.back().color;
	}

	void SortColorStops(std::vector<ParticleEmitterComponent::ColorStop> &p_stops)
	{
		std::stable_sort(p_stops.begin(), p_stops.end(), [](const auto &a, const auto &b) { return a.time < b.time; });
	}

	struct OrbitSample
	{
		glm::vec3 center;
		glm::vec3 axis;
		float radius;
		float angularSpeed;
		float strength;
		bool gravityOnly;
		float softening;
	};

	/// @brief Pulls a particle onto the orbit ring of p_orbit and steers its velocity around the axis
	void ApplyOrbit(const OrbitSample &p_orbit, const glm::vec3 &p_position, glm::vec3 &p_velocity, float p_deltaTime)
	{
		if (p_orbit.gravityOnly)
		{
			// Softened inverse-square attraction towards the centre, without any orbit ring or steering
			const glm::vec3 toCenter = p_orbit.center - p_position;
			const float softening = std::max(p_orbit.softening, 0.01f);
			const float softenedSq = glm::dot(toCenter, toCenter) + softening * softening;
			p_velocity += toCenter * (p_orbit.strength / (softenedSq * std::sqrt(softenedSq)) * p_deltaTime);
			return;
		}

		const glm::vec3 offset = p_position - p_orbit.center;
		const float along = glm::dot(offset, p_orbit.axis);
		const glm::vec3 planar = offset - p_orbit.axis * along;
		const float dist = glm::length(planar);
		const glm::vec3 radial = dist > 1e-4f ? planar / dist : AnyPerpendicular(p_orbit.axis);
		const glm::vec3 tangent = glm::cross(p_orbit.axis, radial);

		// Gain is capped so explicit integration stays stable for large time steps
		const float gain = std::min(p_orbit.strength, 0.5f / p_deltaTime);
		const float tangentialSpeed = glm::dot(p_velocity, tangent);
		const glm::vec3 acceleration =
			(-radial * (dist - p_orbit.radius) - p_orbit.axis * along) * gain +
			tangent * (p_orbit.angularSpeed * dist - tangentialSpeed) * gain;
		p_velocity += acceleration * p_deltaTime;
	}

	/// @brief Smooth, time-varying pseudo-random force field
	glm::vec3 SampleTurbulence(const glm::vec3 &p_position, float p_time, float p_frequency)
	{
		return glm::vec3(
			std::sin(p_position.y * p_frequency + p_time * 1.3f),
			std::sin(p_position.z * p_frequency + 1.7f + p_time * 1.1f),
			std::sin(p_position.x * p_frequency + 3.1f + p_time * 0.9f));
	}
}

namespace
{
	/// @brief The shader a particle stage uses when none is chosen: the renderer's particle shaders by display name
	GUID ParticleShaderOrDefault(GUID p_chosen, const char *p_defaultName)
	{
		if (p_chosen != C_INVALID_GUID)
			return p_chosen;
		if (MetaEntry entry; ResourceManager::GetInstance().GetResourceMetaEntry(std::string(p_defaultName), entry))
			return entry.guid;
		return C_INVALID_GUID;
	}
}

void ParticleEmitterComponent::SetVSGUID(GUID guid)
{
	m_vs = ResourceManager::GetInstance().LoadResource<ShaderResource>(guid);
	if (MetaEntry entry; ResourceManager::GetInstance().GetResourceMetaEntry(guid, entry))
		m_vsDisplayName = entry.displayName;
}

void ParticleEmitterComponent::SetGSGUID(GUID guid)
{
	m_gs = ResourceManager::GetInstance().LoadResource<ShaderResource>(guid);
	if (MetaEntry entry; ResourceManager::GetInstance().GetResourceMetaEntry(guid, entry))
		m_gsDisplayName = entry.displayName;
}

void ParticleEmitterComponent::SetFSGUID(GUID guid)
{
	m_fs = ResourceManager::GetInstance().LoadResource<ShaderResource>(guid);
	if (MetaEntry entry; ResourceManager::GetInstance().GetResourceMetaEntry(guid, entry))
		m_fsDisplayName = entry.displayName;
}

void ParticleEmitterComponent::SetTextureGUID(GUID guid)
{
	m_texture = LoadTrackedResource<Texture2DResource>(TryGetScene(), guid);
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

void ParticleEmitterComponent::RollEmissionRate()
{
	m_activeEmissionRate = m_randomizeEmissionRate
		? RandomFloat(m_emissionRateRange.min, m_emissionRateRange.max)
		: m_emissionRate;
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

glm::vec3 ParticleEmitterComponent::RandomDirectionInCone(const glm::vec3 &p_axis, float p_spreadDegrees)
{
	const float cosMax = std::cos(glm::radians(std::clamp(p_spreadDegrees, 0.0f, 180.0f)));
	const float z = RandomFloat(cosMax, 1.0f);
	const float angle = RandomFloat(0.0f, glm::two_pi<float>());
	const float planar = std::sqrt(std::max(0.0f, 1.0f - z * z));
	const glm::vec3 tangent = AnyPerpendicular(p_axis);
	const glm::vec3 bitangent = glm::cross(p_axis, tangent);
	return tangent * (planar * std::cos(angle)) + bitangent * (planar * std::sin(angle)) + p_axis * z;
}

glm::vec4 ParticleEmitterComponent::EvaluateColorOverLifetime(float normalizedTime) const
{
	return EvaluateGradient(m_colorOverLifetime, normalizedTime);
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

glm::vec3 ParticleEmitterComponent::RandomPointInEmissionVolume()
{
	// Uniform sampling of the region covered by the emission shape
	switch (m_emissionShape)
	{
	case EmissionShape::Sphere:
		return RandomUnitSphere() * (m_shapeRadius * std::cbrt(RandomFloat(0.0f, 1.0f)));
	case EmissionShape::Hemisphere:
	{
		glm::vec3 point = RandomUnitSphere() * (m_shapeRadius * std::cbrt(RandomFloat(0.0f, 1.0f)));
		point.z = std::abs(point.z);
		return point;
	}
	case EmissionShape::Box:
		return {
			RandomFloat(-m_shapeBoxSize.x, m_shapeBoxSize.x) * 0.5f,
			RandomFloat(-m_shapeBoxSize.y, m_shapeBoxSize.y) * 0.5f,
			RandomFloat(-m_shapeBoxSize.z, m_shapeBoxSize.z) * 0.5f
		};
	case EmissionShape::Cone:
		return RandomDirectionForEmission() * RandomFloat(0.0f, m_shapeRadius);
	case EmissionShape::Circle:
	{
		const float angle = RandomFloat(0.0f, glm::two_pi<float>());
		const float distance = m_shapeRadius * std::sqrt(RandomFloat(0.0f, 1.0f));
		return { distance * std::cos(angle), distance * std::sin(angle), 0.0f };
	}
	case EmissionShape::Point:
	default:
		return glm::vec3(0.0f);
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

	if (m_directionMode == DirectionMode::Fixed)
		direction = RandomDirectionInCone(NormalizeOr(m_direction, glm::vec3(0.0f, 1.0f, 0.0f)), m_directionSpread);

	const bool aimed = m_directionMode == DirectionMode::TowardsPoint || m_directionMode == DirectionMode::RandomPointInShape;
	glm::vec3 targetLocal = m_targetPoint;
	if (m_directionMode == DirectionMode::RandomPointInShape)
		targetLocal = RandomPointInEmissionVolume();

	glm::vec3 worldDirection = glm::normalize(direction);
	glm::vec3 worldPosition = localPosition;
	glm::vec3 worldTarget = targetLocal;
	if (GetOwner() && m_simulationSpace == SimulationSpace::World)
	{
		Transform &transform = GetOwner()->GetTransform();
		const glm::mat3 rotation = glm::mat3(transform.GetRotation(Transform::Space::World));
		const glm::vec3 origin = transform.GetPosition(Transform::Space::World);
		worldPosition = origin + rotation * localPosition;
		worldDirection = glm::normalize(rotation * direction);
		worldTarget = origin + rotation * targetLocal;
	}

	if (aimed)
	{
		// Particles spawning exactly on their target keep the shape's own direction
		const glm::vec3 toTarget = worldTarget - worldPosition;
		if (glm::dot(toTarget, toTarget) > 1e-8f)
			worldDirection = glm::normalize(toTarget);
	}

	const float lifetime = RandomFloat(m_lifetimeRange.min, m_lifetimeRange.max);
	const float speed = RandomFloat(m_speedRange.min, m_speedRange.max);
	const glm::vec4 startColor = RandomStartColor();

	Particle particle{};
	particle.position = worldPosition;
	particle.velocity = worldDirection * speed;
	if (m_simulationSpace == SimulationSpace::World)
		particle.velocity += m_emitterVelocity * m_inheritVelocity;
	particle.baseSize = glm::vec2(RandomFloat(m_widthRange.min, m_widthRange.max), RandomFloat(m_heightRange.min, m_heightRange.max));
	particle.size = particle.baseSize;
	particle.rotation = glm::radians(RandomFloat(m_rotationRange.min, m_rotationRange.max));
	particle.angularVelocity = glm::radians(RandomFloat(m_angularVelocityRange.min, m_angularVelocityRange.max));
	particle.color = startColor;
	particle.baseColor = startColor;
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
	m_updateAccumulator += deltaTime * m_timeScale;
	const float updateInterval = m_updateRate > 0.0f ? 1.0f / m_updateRate : 0.0f;
	if (m_updateAccumulator < updateInterval)
	{
		if (m_debugDraw)
			DrawDebugParticles();
		return;
	}

	// Simulate the accumulated time in one step so motion, emission and particle ageing stay consistent
	deltaTime = m_updateAccumulator;
	m_updateAccumulator = 0.0f;

	const glm::vec3 displacement = UpdateOwnerMotion(deltaTime);
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
			RollEmissionRate();
			ResetBurstTriggers();
		}
		else
		{
			m_systemTime = m_duration;
			m_playing = false;
		}
	}

	UpdateBurstEmission();
	const float emissionRate = m_randomizeEmissionRate ? m_activeEmissionRate : m_emissionRate;
	m_emissionAccumulator += emissionRate * deltaTime + m_emissionRateOverDistance * glm::length(displacement);
	uint32_t toEmit = static_cast<uint32_t>(m_emissionAccumulator);
	m_emissionAccumulator -= static_cast<float>(toEmit);
	Emit(toEmit);
}

glm::vec3 ParticleEmitterComponent::UpdateOwnerMotion(float deltaTime)
{
	if (!GetOwner())
	{
		m_emitterVelocity = glm::vec3(0.0f);
		return glm::vec3(0.0f);
	}

	// Scaled deltaTime keeps inherited velocity in the same time base as particle motion
	const glm::vec3 origin = GetOwner()->GetTransform().GetPosition(Transform::Space::World);
	glm::vec3 displacement(0.0f);
	if (m_hasLastOwnerPosition && deltaTime > 0.0f)
	{
		displacement = origin - m_lastOwnerPosition;
		m_emitterVelocity = displacement / deltaTime;
	}
	m_lastOwnerPosition = origin;
	m_hasLastOwnerPosition = true;
	return displacement;
}

void ParticleEmitterComponent::UpdateParticles(float deltaTime)
{
	ZoneScoped;

	const glm::vec3 gravity(0.0f, -9.81f * m_gravityModifier, 0.0f);

	// Particles live in simulation space: world space offsets by the owner's rotation and position, local space is raw
	glm::mat3 simRotation(1.0f);
	glm::vec3 simOrigin(0.0f);
	if (GetOwner() && m_simulationSpace == SimulationSpace::World)
	{
		Transform &transform = GetOwner()->GetTransform();
		simRotation = glm::mat3(transform.GetRotation(Transform::Space::World));
		simOrigin = transform.GetPosition(Transform::Space::World);
	}

	const glm::vec3 acceleration = gravity + simRotation * m_acceleration;

	std::vector<OrbitSample> orbits;
	orbits.reserve(m_orbitPoints.size());
	for (const OrbitPoint &orbit : m_orbitPoints)
	{
		orbits.push_back({
			simOrigin + simRotation * orbit.position,
			simRotation * NormalizeOr(orbit.axis, glm::vec3(0.0f, 1.0f, 0.0f)),
			orbit.radius,
			orbit.angularSpeed,
			orbit.strength,
			orbit.gravityOnly,
			orbit.softening
		});
	}

	m_turbulenceTime += deltaTime;
	const float turbulenceTime = m_turbulenceTime;
	const float turbulenceStrength = m_turbulenceStrength;
	const float turbulenceFrequency = m_turbulenceFrequency;

	#pragma omp parallel for schedule(static)
	for (std::ptrdiff_t i = 0; i < static_cast<std::ptrdiff_t>(m_particles.size()); ++i)
	{
		Particle &particle = m_particles[static_cast<std::size_t>(i)];

		particle.age += deltaTime;
		if (particle.age < particle.lifetime)
		{
			const float normalizedLife = std::clamp(particle.age / particle.lifetime, 0.0f, 1.0f);
			const float velocityMultiplier = EvaluateCurve(m_velocityOverLifetime, normalizedLife);

			particle.velocity += acceleration * deltaTime;
			if (turbulenceStrength > 0.0f)
				particle.velocity += SampleTurbulence(particle.position, turbulenceTime, turbulenceFrequency) * (turbulenceStrength * deltaTime);
			for (const OrbitSample &orbit : orbits)
				ApplyOrbit(orbit, particle.position, particle.velocity, deltaTime);

			particle.position += particle.velocity * velocityMultiplier * deltaTime;
			particle.size = particle.baseSize * EvaluateCurve(m_sizeOverLifetime, normalizedLife);
			particle.color = particle.baseColor * EvaluateColorOverLifetime(normalizedLife);
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

	GenerateParticleMesh();
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

	if (!IsActive())
		return;

	const float deltaTime = Time::Get().GetDeltaTime();
	UpdateEmitter(deltaTime);

	if (m_uiSelected)
	{
		DrawDebugEmitter();
	}
	m_uiSelected = false;
}

void ParticleEmitterComponent::DrawDebugParticles()
{
	ZoneScoped;

	using Droplet::Graphics::Debug::DebugDraw;

	Transform &transform = GetTransform();
	glm::mat4 worldMatrix = transform.GetMatrix(Transform::Space::World);

	for (const Particle &particle : m_particles)
	{
		const float radius = std::max(0.001f, std::max(particle.size.x, particle.size.y) * 0.5f);

		glm::vec3 pos = particle.position;
		if (m_simulationSpace == SimulationSpace::Local)
		{
			pos = glm::vec3(worldMatrix * glm::vec4(pos, 1.0f));
		}

		DebugDraw::DrawSphere(
			Sphere(pos, radius),
			particle.color,
			0
		);
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

	const glm::mat3 ownerRotation = GetOwner() ? glm::mat3(GetOwner()->GetTransform().GetRotation(Transform::Space::World)) : glm::mat3(1.0f);
	const glm::mat4 worldMatrix = GetOwner() ? GetOwner()->GetTransform().GetMatrix(Transform::Space::World) : glm::mat4(1.0f);

	if (m_directionMode == DirectionMode::Fixed)
	{
		const glm::vec3 dir = NormalizeOr(ownerRotation * NormalizeOr(m_direction, glm::vec3(0.0f, 1.0f, 0.0f)), forward);
		const glm::vec4 directionColor(1.0f, 0.6f, 0.2f, 0.9f);
		const glm::vec3 tip = position + dir * radius;
		DebugDraw::DrawRay(position, dir * radius, directionColor, false);

		if (m_directionSpread > 0.0f)
		{
			const float baseRadius = radius * std::tan(glm::radians(std::min(m_directionSpread, 89.0f)));
			const glm::vec3 perp = AnyPerpendicular(dir);
			const glm::vec3 perp2 = glm::cross(dir, perp);
			DebugDraw::DrawCircle(tip, dir, baseRadius, directionColor, 24, false, true);
			DebugDraw::DrawLine(position, tip + perp * baseRadius, directionColor, false);
			DebugDraw::DrawLine(position, tip - perp * baseRadius, directionColor, false);
			DebugDraw::DrawLine(position, tip + perp2 * baseRadius, directionColor, false);
			DebugDraw::DrawLine(position, tip - perp2 * baseRadius, directionColor, false);
		}
	}

	if (m_directionMode == DirectionMode::TowardsPoint)
	{
		const glm::vec4 targetColor(0.3f, 1.0f, 0.4f, 0.9f);
		const glm::vec3 target = m_simulationSpace == SimulationSpace::World
			? position + ownerRotation * m_targetPoint
			: glm::vec3(worldMatrix * glm::vec4(m_targetPoint, 1.0f));
		DebugDraw::DrawLine(position, target, targetColor, false);
		DebugDraw::DrawSphere(Sphere(target, 0.1f * radiusScale), targetColor, 1, false);
	}

	// Orbit centers and axes follow the same transform rules as the simulation (see UpdateParticles)
	const glm::vec4 orbitColor(1.0f, 0.4f, 0.9f, 0.8f);
	const glm::vec4 gravityColor(0.3f, 0.9f, 1.0f, 0.8f);
	for (const OrbitPoint &orbit : m_orbitPoints)
	{
		glm::vec3 center;
		glm::vec3 axis = NormalizeOr(orbit.axis, glm::vec3(0.0f, 1.0f, 0.0f));
		float orbitRadius = orbit.radius;
		if (m_simulationSpace == SimulationSpace::World)
		{
			center = position + ownerRotation * orbit.position;
			axis = ownerRotation * axis;
		}
		else
		{
			center = glm::vec3(worldMatrix * glm::vec4(orbit.position, 1.0f));
			axis = NormalizeOr(glm::mat3(worldMatrix) * axis, axis);
			orbitRadius *= radiusScale;
		}

		if (orbit.gravityOnly)
		{
			DebugDraw::DrawSphere(Sphere(center, 0.1f * radiusScale), gravityColor, 1, false);
			continue;
		}
		DebugDraw::DrawCircle(center, axis, orbitRadius, orbitColor, 32, false, true);
		DebugDraw::DrawSphere(Sphere(center, 0.05f * radiusScale), orbitColor, 1, false);
	}
}

void ParticleEmitterComponent::QueueRenderImpl()
{
	if (GetOwner() && GetOwner()->GetScene())
		GetOwner()->GetScene()->QueueRenderable(shared_from_this());
}

RenderData ParticleEmitterComponent::GetRenderDataImpl()
{
	Common::RenderData renderData{};
	renderData.dynamicMesh = m_particleMesh;
	renderData.texData.GUID = m_texture.GetGUID();
	renderData.vsData.guid = ParticleShaderOrDefault(m_vs.GetGUID(), "Particle.VS");
	renderData.gsData.guid = ParticleShaderOrDefault(m_gs.GetGUID(), "Particle.GS");
	renderData.fsData.guid = ParticleShaderOrDefault(m_fs.GetGUID(), "Particle.FS");
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

void ParticleEmitterComponent::UpdateBounds(glm::vec3 p_min, glm::vec3 p_max)
{
	// Clamp the min and max values to ensure they are valid
	p_min = glm::min(p_min, p_max);

	OBB obb{};

	obb.center = (p_min + p_max) * 0.5f;
	obb.extents = (p_max - p_min) * 0.5f;

	m_bounds.SetOBB(obb);
	GetTransform().MakePhysicsDirty();
}

nlohmann::json ParticleEmitterComponent::SerializeImpl()
{
	nlohmann::json json;
	json["duration"] = m_duration;
	json["looping"] = m_looping;
	json["playOnAwake"] = m_playOnAwake;
	json["startDelay"] = m_startDelay;
	json["timeScale"] = m_timeScale;
	json["updateRate"] = m_updateRate;
	json["maxParticles"] = m_maxParticles;
	json["emissionRate"] = m_emissionRate;
	json["emissionRateOverDistance"] = m_emissionRateOverDistance;
	json["randomizeEmissionRate"] = m_randomizeEmissionRate;
	json["emissionRateRange"] = { m_emissionRateRange.min, m_emissionRateRange.max };
	json["inheritVelocity"] = m_inheritVelocity;
	json["gravityModifier"] = m_gravityModifier;
	json["emissionShape"] = static_cast<int>(m_emissionShape);
	json["simulationSpace"] = static_cast<int>(m_simulationSpace);
	json["startColorMode"] = static_cast<int>(m_startColorMode);
	json["shapeRadius"] = m_shapeRadius;
	json["shapeBoxSize"] = { m_shapeBoxSize.x, m_shapeBoxSize.y, m_shapeBoxSize.z };
	json["coneAngle"] = m_coneAngle;
	json["transparent"] = m_transparent;
	json["billboardOffset"] = { m_billboardOffset.x, m_billboardOffset.y };
	json["debugDraw"] = m_debugDraw;
	json["textureGUID"] = GetTextureGUID();
	json["textureDisplayName"] = m_textureDisplayName;
	json["vsGUID"] = m_vs.GetGUID();
	json["vsDisplayName"] = m_vsDisplayName;
	json["gsGUID"] = m_gs.GetGUID();
	json["gsDisplayName"] = m_gsDisplayName;
	json["fsGUID"] = m_fs.GetGUID();
	json["fsDisplayName"] = m_fsDisplayName;

	json["lifetimeRange"] = { m_lifetimeRange.min, m_lifetimeRange.max };
	json["speedRange"] = { m_speedRange.min, m_speedRange.max };
	json["widthRange"] = { m_widthRange.min, m_widthRange.max };
	json["heightRange"] = { m_heightRange.min, m_heightRange.max };
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

	json["randomSeed"] = m_randomSeed;
	json["randomSeedEachPlay"] = m_randomSeedEachPlay;
	json["directionMode"] = static_cast<int>(m_directionMode);
	json["direction"] = { m_direction.x, m_direction.y, m_direction.z };
	json["directionSpread"] = m_directionSpread;
	json["targetPoint"] = { m_targetPoint.x, m_targetPoint.y, m_targetPoint.z };
	json["acceleration"] = { m_acceleration.x, m_acceleration.y, m_acceleration.z };
	json["turbulenceStrength"] = m_turbulenceStrength;
	json["turbulenceFrequency"] = m_turbulenceFrequency;

	json["colorOverLifetime"] = nlohmann::json::array();
	for (const ColorStop &stop : m_colorOverLifetime)
		json["colorOverLifetime"].push_back({ { "time", stop.time }, { "color", { stop.color.r, stop.color.g, stop.color.b, stop.color.a } } });

	json["orbitPoints"] = nlohmann::json::array();
	for (const OrbitPoint &orbit : m_orbitPoints)
		json["orbitPoints"].push_back({
			{ "position", { orbit.position.x, orbit.position.y, orbit.position.z } },
			{ "axis", { orbit.axis.x, orbit.axis.y, orbit.axis.z } },
			{ "radius", orbit.radius },
			{ "angularSpeed", orbit.angularSpeed },
			{ "strength", orbit.strength },
			{ "gravityOnly", orbit.gravityOnly },
			{ "softening", orbit.softening } });
	return json;
}

void ParticleEmitterComponent::DeserializeImpl(const nlohmann::json p_compJson)
{
	if (p_compJson.contains("duration")) SetDuration(p_compJson["duration"].get<float>());
	if (p_compJson.contains("looping")) SetLooping(p_compJson["looping"].get<bool>());
	if (p_compJson.contains("playOnAwake")) SetPlayOnAwake(p_compJson["playOnAwake"].get<bool>());
	if (p_compJson.contains("startDelay")) SetStartDelay(p_compJson["startDelay"].get<float>());
	if (p_compJson.contains("timeScale")) SetTimeScale(p_compJson["timeScale"].get<float>());
	if (p_compJson.contains("updateRate")) SetUpdateRate(p_compJson["updateRate"].get<float>());
	if (p_compJson.contains("maxParticles")) SetMaxParticles(p_compJson["maxParticles"].get<uint32_t>());
	if (p_compJson.contains("emissionRate")) SetEmissionRate(p_compJson["emissionRate"].get<float>());
	if (p_compJson.contains("emissionRateOverDistance")) SetEmissionRateOverDistance(p_compJson["emissionRateOverDistance"].get<float>());
	if (p_compJson.contains("emissionRateRange"))
	{
		const auto &v = p_compJson["emissionRateRange"];
		SetEmissionRateRange({ v[0].get<float>(), v[1].get<float>() });
	}
	if (p_compJson.contains("randomizeEmissionRate")) SetRandomizeEmissionRate(p_compJson["randomizeEmissionRate"].get<bool>());
	if (p_compJson.contains("inheritVelocity")) SetInheritVelocity(p_compJson["inheritVelocity"].get<float>());
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
	if (p_compJson.contains("billboardOffset"))
	{
		const auto &v = p_compJson["billboardOffset"];
		SetBillboardOffset(glm::vec2(v[0].get<float>(), v[1].get<float>()));
	}
	if (p_compJson.contains("debugDraw")) SetDebugDraw(p_compJson["debugDraw"].get<bool>());
	if (p_compJson.contains("textureGUID")) SetTextureGUID(p_compJson["textureGUID"].get<GUID>());
	if (p_compJson.contains("textureDisplayName")) m_textureDisplayName = p_compJson["textureDisplayName"].get<std::string>();
	if (p_compJson.contains("vsGUID")) SetVSGUID(p_compJson["vsGUID"].get<GUID>());
	if (p_compJson.contains("vsDisplayName")) m_vsDisplayName = p_compJson["vsDisplayName"].get<std::string>();
	if (p_compJson.contains("gsGUID")) SetGSGUID(p_compJson["gsGUID"].get<GUID>());
	if (p_compJson.contains("gsDisplayName")) m_gsDisplayName = p_compJson["gsDisplayName"].get<std::string>();
	if (p_compJson.contains("fsGUID")) SetFSGUID(p_compJson["fsGUID"].get<GUID>());
	if (p_compJson.contains("fsDisplayName")) m_fsDisplayName = p_compJson["fsDisplayName"].get<std::string>();

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
	if (p_compJson.contains("widthRange"))
	{
		const auto &v = p_compJson["widthRange"];
		SetWidthRange({ v[0].get<float>(), v[1].get<float>() });
	}
	if (p_compJson.contains("heightRange"))
	{
		const auto &v = p_compJson["heightRange"];
		SetHeightRange({ v[0].get<float>(), v[1].get<float>() });
	}
	// Scenes saved before width/height existed have a single uniform size range
	if (p_compJson.contains("sizeRange") && !p_compJson.contains("widthRange") && !p_compJson.contains("heightRange"))
	{
		const auto &v = p_compJson["sizeRange"];
		SetWidthRange({ v[0].get<float>(), v[1].get<float>() });
		SetHeightRange({ v[0].get<float>(), v[1].get<float>() });
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

	if (p_compJson.contains("randomSeed")) SetRandomSeed(p_compJson["randomSeed"].get<uint32_t>());
	if (p_compJson.contains("randomSeedEachPlay")) SetRandomSeedEachPlay(p_compJson["randomSeedEachPlay"].get<bool>());
	if (p_compJson.contains("directionMode"))
		m_directionMode = static_cast<DirectionMode>(p_compJson["directionMode"].get<int>());
	if (p_compJson.contains("direction"))
	{
		const auto &v = p_compJson["direction"];
		m_direction = glm::vec3(v[0].get<float>(), v[1].get<float>(), v[2].get<float>());
	}
	if (p_compJson.contains("directionSpread")) SetDirectionSpread(p_compJson["directionSpread"].get<float>());
	if (p_compJson.contains("targetPoint"))
	{
		const auto &v = p_compJson["targetPoint"];
		SetTargetPoint(glm::vec3(v[0].get<float>(), v[1].get<float>(), v[2].get<float>()));
	}
	if (p_compJson.contains("acceleration"))
	{
		const auto &v = p_compJson["acceleration"];
		m_acceleration = glm::vec3(v[0].get<float>(), v[1].get<float>(), v[2].get<float>());
	}
	if (p_compJson.contains("turbulenceStrength")) SetTurbulenceStrength(p_compJson["turbulenceStrength"].get<float>());
	if (p_compJson.contains("turbulenceFrequency")) SetTurbulenceFrequency(p_compJson["turbulenceFrequency"].get<float>());

	if (p_compJson.contains("colorOverLifetime"))
	{
		std::vector<ColorStop> stops;
		for (const nlohmann::json &stop : p_compJson["colorOverLifetime"])
		{
			const auto &c = stop["color"];
			stops.push_back({ stop["time"].get<float>(), glm::vec4(c[0].get<float>(), c[1].get<float>(), c[2].get<float>(), c[3].get<float>()) });
		}
		SetColorOverLifetime(std::move(stops));
	}

	ClearOrbitPoints();
	if (p_compJson.contains("orbitPoints"))
		for (const nlohmann::json &orbit : p_compJson["orbitPoints"])
		{
			const auto &p = orbit["position"];
			const auto &a = orbit["axis"];
			OrbitPoint point;
			point.position = glm::vec3(p[0].get<float>(), p[1].get<float>(), p[2].get<float>());
			point.axis = glm::vec3(a[0].get<float>(), a[1].get<float>(), a[2].get<float>());
			point.radius = orbit["radius"].get<float>();
			point.angularSpeed = orbit["angularSpeed"].get<float>();
			point.strength = orbit["strength"].get<float>();
			point.gravityOnly = orbit.value("gravityOnly", false);
			point.softening = orbit.value("softening", 0.5f);
			AddOrbitPoint(point);
		}
}

void ParticleEmitterComponent::RenderUIImpl()
{
#ifdef _EDITOR
	using namespace ImGui;

	m_uiSelected = true;

	PushID(this);
	PushItemWidth(GetFontSize() * 12.0f);

	auto ResetOnRightClick = [](auto &&reset) {
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Reset"))
				reset();
			ImGui::EndPopup();
		}
	};

	auto RenderRange = [&ResetOnRightClick](const char *label, const char *tooltip, FloatRange &range, float speed, float minLimit, float maxLimit, FloatRange defaults) {
		float values[2] = { range.min, range.max };
		if (ImGui::DragFloat2(label, values, speed, minLimit, maxLimit))
		{
			// Only the component that changed may move, and it must not cross the other one
			if (values[0] != range.min)
				values[0] = std::min(values[0], values[1]);
			else
				values[1] = std::max(values[1], values[0]);
			range.min = values[0];
			range.max = values[1];
		}
		ImGui::SetItemTooltip("%s", tooltip);
		ResetOnRightClick([&] { range = defaults; });
	};

	if (Button(m_playing ? "Pause" : "Play"))
	{
		if (m_playing)
			Pause();
		else
			Play();
	}
	SetItemTooltip("Start the emitter, or pause it while it is playing.");
	SameLine();
	if (Button("Stop"))
		Stop(true);
	SetItemTooltip("Stop the emitter and clear all particles.");
	SameLine();
	if (Button("Burst"))
		Emit(std::max(1u, m_bursts.empty() ? 30u : m_bursts.front().count));
	SetItemTooltip("Emit one burst of particles immediately.");

	auto RenderGradientEditor = [](const char *label, std::vector<ColorStop> &stops) {
		ImGuiStorage *storage = ImGui::GetStateStorage();
		const ImGuiID selectedID = ImGui::GetID("selectedColorStop");
		int selected = storage->GetInt(selectedID, -1);
		if (selected >= static_cast<int>(stops.size()))
			selected = -1;

		const float barWidth = std::max(1.0f, GetContentRegionAvail().x);
		const float barHeight = 24.0f;
		const float handleRow = 14.0f;
		const ImVec2 origin = GetCursorScreenPos();
		const auto timeToX = [&](float t) { return origin.x + barWidth * t; };
		const float mouseT = std::clamp((GetMousePos().x - origin.x) / barWidth, 0.0f, 1.0f);

		ImDrawList *drawList = GetWindowDrawList();
		constexpr int segments = 64;
		for (int i = 0; i < segments; ++i)
		{
			const float t0 = static_cast<float>(i) / segments;
			const float t1 = static_cast<float>(i + 1) / segments;
			const glm::vec4 c = EvaluateGradient(stops, (t0 + t1) * 0.5f);
			drawList->AddRectFilled(
				ImVec2(timeToX(t0), origin.y),
				ImVec2(timeToX(t1), origin.y + barHeight),
				ColorConvertFloat4ToU32(ImVec4(c.r, c.g, c.b, c.a)));
		}
		drawList->AddRect(origin, ImVec2(origin.x + barWidth, origin.y + barHeight), IM_COL32(255, 255, 255, 60));

		InvisibleButton(label, ImVec2(barWidth, barHeight + handleRow));
		SetItemTooltip("Colour across the particle's lifetime, from birth (left) to death (right).");
		const bool hovered = IsItemHovered();

		if (IsItemActivated())
		{
			selected = -1;
			float bestDistance = 8.0f;
			for (int i = 0; i < static_cast<int>(stops.size()); ++i)
			{
				const float distance = std::abs(GetMousePos().x - timeToX(stops[i].time));
				if (distance < bestDistance)
				{
					bestDistance = distance;
					selected = i;
				}
			}
		}

		if (hovered && IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			stops.push_back({ mouseT, EvaluateGradient(stops, mouseT) });
			SortColorStops(stops);
			selected = -1;
		}

		if (selected >= 0 && IsItemActive() && IsMouseDragging(ImGuiMouseButton_Left))
			stops[selected].time = mouseT;

		if (selected >= 0 && IsItemDeactivated())
		{
			SortColorStops(stops);
			selected = -1;
		}
		storage->SetInt(selectedID, selected);

		for (int i = 0; i < static_cast<int>(stops.size()); ++i)
		{
			const glm::vec4 &c = stops[i].color;
			const ImVec2 center(timeToX(stops[i].time), origin.y + barHeight + handleRow * 0.5f);
			drawList->AddRectFilled(
				ImVec2(center.x - 5.0f, center.y - 5.0f),
				ImVec2(center.x + 5.0f, center.y + 5.0f),
				ColorConvertFloat4ToU32(ImVec4(c.r, c.g, c.b, 1.0f)));
			drawList->AddRect(
				ImVec2(center.x - 5.0f, center.y - 5.0f),
				ImVec2(center.x + 5.0f, center.y + 5.0f),
				i == selected ? IM_COL32(255, 255, 255, 255) : IM_COL32(0, 0, 0, 255),
				0.0f, 0, i == selected ? 2.0f : 1.0f);
		}
	};

	auto RenderColorStopList = [](std::vector<ColorStop> &stops) {
		std::size_t removeIndex = stops.size();
		bool changed = false;
		for (std::size_t i = 0; i < stops.size(); ++i)
		{
			PushID(static_cast<int>(i));
			float time = stops[i].time;
			if (ImGui::SliderFloat("Time", &time, 0.0f, 1.0f))
			{
				stops[i].time = time;
				changed = true;
			}
			ImGui::SetItemTooltip("Position of this stop along the lifetime, from 0 for birth to 1 for death.");
			ImGui::ColorEdit4("Color", &stops[i].color.r, ImGuiColorEditFlags_AlphaBar);
			ImGui::SetItemTooltip("Colour and alpha at this stop.");
			ImGui::BeginDisabled(stops.size() <= 1);
			if (ImGui::Button("Remove"))
				removeIndex = i;
			ImGui::SetItemTooltip("Remove this colour stop. At least one stop is kept.");
			ImGui::EndDisabled();
			ImGui::Separator();
			PopID();
		}
		if (removeIndex < stops.size())
		{
			stops.erase(stops.begin() + removeIndex);
			changed = true;
		}
		if (ImGui::Button("Add Stop"))
		{
			stops.push_back({ 1.0f, stops.empty() ? glm::vec4(1.0f) : stops.back().color });
			changed = true;
		}
		ImGui::SetItemTooltip("Add a stop at the end of the lifetime.");
		ImGui::SameLine();
		if (ImGui::Button("Reset"))
		{
			stops = { { 0.0f, glm::vec4(1.0f) }, { 1.0f, glm::vec4(1.0f) } };
			changed = true;
		}
		ImGui::SetItemTooltip("Restore the default gradient: white, fully opaque.");
		if (changed)
			SortColorStops(stops);
	};

	if (CollapsingHeader("General", ImGuiTreeNodeFlags_DefaultOpen))
	{
		Checkbox("Looping", &m_looping);
		SetItemTooltip("Restart the emitter automatically when its duration ends.");
		ResetOnRightClick([&] { SetLooping(true); });
		Checkbox("Play On Awake", &m_playOnAwake);
		SetItemTooltip("Start emitting as soon as the scene loads.");
		ResetOnRightClick([&] { SetPlayOnAwake(true); });
		DragFloat("Duration", &m_duration, 0.05f, 0.001f, 3600.0f);
		SetItemTooltip("Length of one emission cycle, in seconds. Bursts are timed within it.");
		ResetOnRightClick([&] { SetDuration(5.0f); });
		DragFloat("Start Delay", &m_startDelay, 0.025f, 0.0f, 60.0f);
		SetItemTooltip("Seconds to wait after playing before the first particle is emitted.");
		ResetOnRightClick([&] { SetStartDelay(0.0f); });
		float timeScale = m_timeScale;
		if (DragFloat("Time Scale", &timeScale, 0.005f, 0.0f))
			SetTimeScale(timeScale);
		SetItemTooltip("Speed multiplier for the simulation. 0 freezes it, 2 runs at double speed.");
		ResetOnRightClick([&] { SetTimeScale(1.0f); });
		float updateRate = m_updateRate;
		if (DragFloat("Update Rate (Hz)", &updateRate, 0.25f, 0.0f, 1000.0f))
			SetUpdateRate(updateRate);
		SetItemTooltip("Simulation updates per second, capped to once per frame. 0: every frame.");
		ResetOnRightClick([&] { SetUpdateRate(0.0f); });

		int maxParticles = static_cast<int>(m_maxParticles);
		if (DragInt("Max Particles", &maxParticles, 0.5f, 1, 1000000))
			SetMaxParticles(static_cast<uint32_t>(maxParticles));
		SetItemTooltip("Maximum number of particles alive at once.");
		ResetOnRightClick([&] { SetMaxParticles(1000u); });

		int seed = static_cast<int>(m_randomSeed & 0x7FFFFFFFu);
		if (DragInt("Random Seed", &seed, 0.5f, 0, 0x7FFFFFFF))
			SetRandomSeed(static_cast<uint32_t>(seed));
		SetItemTooltip("Seed for all random values. The same seed gives the same result.");
		ResetOnRightClick([&] { SetRandomSeed(0xD20B1E7u); });
		SameLine();
		if (Button("Reroll"))
			SetRandomSeed(static_cast<uint32_t>(m_random()) & 0x7FFFFFFFu);
		SetItemTooltip("Pick a new random seed.");
		Checkbox("Random Seed Each Play", &m_randomSeedEachPlay);
		SetItemTooltip("Use a new random seed every time the emitter plays. Turn off to replay the same result from the seed above.");
		ResetOnRightClick([&] { SetRandomSeedEachPlay(false); });

		const char *spaceNames[] = { "Local", "World" };
		int space = static_cast<int>(m_simulationSpace);
		if (Combo("Simulation Space", &space, spaceNames, IM_ARRAYSIZE(spaceNames)))
			m_simulationSpace = static_cast<SimulationSpace>(space);
		SetItemTooltip("Local: particles move with the emitter. World: particles stay where they were emitted.");
		ResetOnRightClick([&] { SetSimulationSpace(SimulationSpace::World); });
	}

	if (CollapsingHeader("Emission", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (Checkbox("Randomize Emission Rate", &m_randomizeEmissionRate))
			RollEmissionRate();
		SetItemTooltip("Roll the emission rate from a range instead of using a fixed value.");
		ResetOnRightClick([&] { SetRandomizeEmissionRate(false); });
		if (m_randomizeEmissionRate)
		{
			const FloatRange before = m_emissionRateRange;
			RenderRange("Emission Rate", "Range the emission rate is rolled from each time the emitter starts or loops.", m_emissionRateRange, 0.25f, 0.0f, 100000.0f, { 5.0f, 15.0f });
			if (before.min != m_emissionRateRange.min || before.max != m_emissionRateRange.max)
				RollEmissionRate();
			TextDisabled("A new rate is rolled each time the emitter starts or loops.");
		}
		else
		{
			DragFloat("Emission Rate", &m_emissionRate, 0.25f, 0.0f, 100000.0f);
			SetItemTooltip("Particles emitted per second.");
			ResetOnRightClick([&] { SetEmissionRate(10.0f); });
		}
		DragFloat("Emission Rate Over Distance", &m_emissionRateOverDistance, 0.025f, 0.0f, 100000.0f);
		SetItemTooltip("Extra particles emitted per world unit the emitter travels, on top of the rate.");
		ResetOnRightClick([&] { SetEmissionRateOverDistance(0.0f); });
		TextDisabled("Particles per world unit the emitter travels.");

		if (TreeNodeEx(("Bursts (" + std::to_string(m_bursts.size()) + ")###Bursts").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			std::size_t removeIndex = m_bursts.size();
			for (std::size_t i = 0; i < m_bursts.size(); ++i)
			{
				PushID(static_cast<int>(i));
				const std::string label = "Burst " + std::to_string(i + 1);
				if (TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
				{
					float time = m_bursts[i].time;
					int count = static_cast<int>(m_bursts[i].count);
					if (DragFloat("Time", &time, 0.025f, 0.0f, m_duration))
						m_bursts[i].time = std::max(0.0f, time);
					SetItemTooltip("Time in the emission cycle, in seconds, when this burst fires.");
					ResetOnRightClick([&] { m_bursts[i].time = 0.0f; });
					if (DragInt("Count", &count, 0.5f, 1, 1000000))
						m_bursts[i].count = static_cast<uint32_t>(std::max(1, count));
					SetItemTooltip("Number of particles emitted by this burst.");
					ResetOnRightClick([&] { m_bursts[i].count = 30u; });
					if (Button("Remove"))
						removeIndex = i;
					SetItemTooltip("Remove this burst.");
					TreePop();
				}
				PopID();
			}
			if (removeIndex < m_bursts.size())
				m_bursts.erase(m_bursts.begin() + removeIndex);
			if (Button("Add Burst"))
				AddBurst(0.0f, 30);
			SetItemTooltip("Add a burst at time 0 with 30 particles.");
			TreePop();
		}
	}
	if (CollapsingHeader("Start Values", ImGuiTreeNodeFlags_DefaultOpen))
	{
		RenderRange("Lifetime", "Range particle lifetimes are picked from, in seconds.", m_lifetimeRange, 0.025f, 0.001f, 3600.0f, { 1.0f, 2.0f });
		RenderRange("Speed", "Range initial particle speeds are picked from, in units per second.", m_speedRange, 0.025f, -1000.0f, 1000.0f, { 5.0f, 5.0f });
		if (m_simulationSpace == SimulationSpace::World)
		{
			float inheritVelocity = m_inheritVelocity;
			if (DragFloat("Inherit Velocity", &inheritVelocity, 0.005f, 0.0f, 100.0f))
				SetInheritVelocity(inheritVelocity);
			SetItemTooltip("Fraction of the emitter's velocity that new particles inherit. World space only.");
			ResetOnRightClick([&] { SetInheritVelocity(0.0f); });
		}
		else
		{
			TextDisabled("Inherit Velocity requires World simulation space.");
		}
		RenderRange("Width", "Range particle widths are picked from, in world units.", m_widthRange, 0.005f, 0.0f, 1000.0f, { 0.1f, 0.1f });
		RenderRange("Height", "Range particle heights are picked from, in world units.", m_heightRange, 0.005f, 0.0f, 1000.0f, { 0.1f, 0.1f });
		RenderRange("Rotation", "Range initial particle rotations are picked from, in degrees.", m_rotationRange, 0.5f, -3600.0f, 3600.0f, { 0.0f, 360.0f });
		RenderRange("Angular Velocity", "Range particle spin speeds are picked from, in degrees per second.", m_angularVelocityRange, 0.5f, -3600.0f, 3600.0f, { 0.0f, 0.0f });
		ColorEdit4("Start Color Min", &m_startColorRange.min.r);
		SetItemTooltip("Lower colour bound. Each particle's starting colour is picked between Min and Max.");
		ResetOnRightClick([&] { m_startColorRange.min = glm::vec4(1.0f); });
		ColorEdit4("Start Color Max", &m_startColorRange.max.r);
		SetItemTooltip("Upper colour bound. Each particle's starting colour is picked between Min and Max.");
		ResetOnRightClick([&] { m_startColorRange.max = glm::vec4(1.0f); });
		const char *startColorModeNames[] = { "Random Per Component", "Lerp", "Spherical Lerp" };
		int startColorMode = static_cast<int>(m_startColorMode);
		if (Combo("Start Color Mode", &startColorMode, startColorModeNames, IM_ARRAYSIZE(startColorModeNames)))
			m_startColorMode = static_cast<StartColorMode>(startColorMode);
		SetItemTooltip("How colours are picked between Min and Max. Random Per Component picks each channel independently; Lerp blends linearly; Spherical Lerp blends along the colour direction.");
		ResetOnRightClick([&] { SetStartColorMode(StartColorMode::Lerp); });
	}


	if (CollapsingHeader("Shape", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const char *shapeNames[] = { "Point", "Sphere", "Hemisphere", "Box", "Cone", "Circle" };
		int shape = static_cast<int>(m_emissionShape);
		if (Combo("Emission Shape", &shape, shapeNames, IM_ARRAYSIZE(shapeNames)))
			SetEmissionShape(static_cast<EmissionShape>(shape));
		SetItemTooltip("Region particles spawn in. The Shape direction mode uses its directions.");
		ResetOnRightClick([&] { SetEmissionShape(EmissionShape::Point); });

		if (m_emissionShape == EmissionShape::Sphere || m_emissionShape == EmissionShape::Hemisphere ||
			m_emissionShape == EmissionShape::Cone || m_emissionShape == EmissionShape::Circle)
		{
			float radius = m_shapeRadius;
			if (DragFloat("Radius", &radius, 0.025f, 0.0f, 1000.0f))
				SetShapeRadius(radius);
			SetItemTooltip("Radius of the shape, in world units.");
			ResetOnRightClick([&] { SetShapeRadius(1.0f); });
		}
		if (m_emissionShape == EmissionShape::Box)
		{
			glm::vec3 boxSize = m_shapeBoxSize;
			if (DragFloat3("Box Size", &boxSize.x, 0.025f, 0.0f, 1000.0f))
				SetShapeBoxSize(boxSize);
			SetItemTooltip("Full width, height and depth of the box, in world units.");
			ResetOnRightClick([&] { SetShapeBoxSize(glm::vec3(1.0f)); });
		}
		if (m_emissionShape == EmissionShape::Cone)
		{
			float angle = m_coneAngle;
			if (DragFloat("Cone Angle", &angle, 0.25f, 0.0f, 180.0f))
				SetConeAngle(angle);
			SetItemTooltip("Half-angle of the cone, in degrees, measured from its axis to its edge.");
			ResetOnRightClick([&] { SetConeAngle(25.0f); });
		}
	}

	if (CollapsingHeader("Direction"))
	{
		const char *directionNames[] = { "Shape", "Fixed", "Towards Point", "Random Point In Shape" };
		int directionMode = static_cast<int>(m_directionMode);
		if (Combo("Direction Mode", &directionMode, directionNames, IM_ARRAYSIZE(directionNames)))
			SetDirectionMode(static_cast<DirectionMode>(directionMode));
		SetItemTooltip("Shape: use the shape's own directions. Fixed: one direction with spread. Towards Point: aim at a point. Random Point In Shape: aim each particle at a random point inside the shape.");
		ResetOnRightClick([&] { SetDirectionMode(DirectionMode::Shape); });

		if (m_directionMode == DirectionMode::Fixed)
		{
			glm::vec3 direction = m_direction;
			if (DragFloat3("Direction", &direction.x, 0.005f))
				SetDirection(direction);
			SetItemTooltip("Base travel direction, in emitter-local space.");
			ResetOnRightClick([&] { SetDirection(glm::vec3(0.0f, 1.0f, 0.0f)); });
			float spread = m_directionSpread;
			if (DragFloat("Spread", &spread, 0.25f, 0.0f, 180.0f))
				SetDirectionSpread(spread);
			SetItemTooltip("Random angle, in degrees, that particles may deviate from the base direction.");
			ResetOnRightClick([&] { SetDirectionSpread(0.0f); });
		}
		else if (m_directionMode == DirectionMode::TowardsPoint)
		{
			glm::vec3 target = m_targetPoint;
			if (DragFloat3("Target Point", &target.x, 0.025f))
				SetTargetPoint(target);
			SetItemTooltip("Point, in emitter-local space, that particles are aimed at.");
			ResetOnRightClick([&] { SetTargetPoint(glm::vec3(0.0f, 5.0f, 0.0f)); });
			TextDisabled("Particles are aimed at this point in emitter-local space.");
		}
		else if (m_directionMode == DirectionMode::RandomPointInShape)
		{
			TextDisabled("Each particle is aimed at a random point inside the emission shape.");
		}
		else
		{
			TextDisabled("Particles use the emission shape's own directions.");
		}
	}

	if (CollapsingHeader("Forces"))
	{
		DragFloat("Gravity Modifier", &m_gravityModifier, 0.025f, -10.0f, 10.0f);
		SetItemTooltip("Scales gravity. 0 disables it, negative values pull particles upward.");
		ResetOnRightClick([&] { SetGravityModifier(0.0f); });

		glm::vec3 acceleration = m_acceleration;
		if (DragFloat3("Acceleration", &acceleration.x, 0.025f, -100.0f, 100.0f))
			SetAcceleration(acceleration);
		SetItemTooltip("Constant acceleration added to every particle's velocity each second.");
		ResetOnRightClick([&] { SetAcceleration(glm::vec3(0.0f)); });

		float turbulenceStrength = m_turbulenceStrength;
		if (DragFloat("Turbulence Strength", &turbulenceStrength, 0.025f, 0.0f, 100.0f))
			SetTurbulenceStrength(turbulenceStrength);
		SetItemTooltip("Strength of a noise-based force that pushes particles around.");
		ResetOnRightClick([&] { SetTurbulenceStrength(0.0f); });
		float turbulenceFrequency = m_turbulenceFrequency;
		if (DragFloat("Turbulence Frequency", &turbulenceFrequency, 0.025f, 0.0f, 100.0f))
			SetTurbulenceFrequency(turbulenceFrequency);
		SetItemTooltip("How quickly the turbulence changes across space and time.");
		ResetOnRightClick([&] { SetTurbulenceFrequency(1.0f); });
	}

	if (CollapsingHeader("Orbit Points"))
	{
		TextDisabled("Orbit points form a ring around their axis. Gravity Only points attract particles instead.");
		std::size_t removeIndex = m_orbitPoints.size();
		for (std::size_t i = 0; i < m_orbitPoints.size(); ++i)
		{
			PushID(static_cast<int>(i));
			const std::string label = "Orbit " + std::to_string(i + 1);
			if (TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				OrbitPoint &orbit = m_orbitPoints[i];
				DragFloat3("Position", &orbit.position.x, 0.025f);
				SetItemTooltip("Centre of the orbit point, in emitter-local space.");
				ResetOnRightClick([&] { orbit.position = glm::vec3(0.0f); });
				Checkbox("Gravity Only", &orbit.gravityOnly);
				SetItemTooltip("Attract particles like a gravity well instead of orbiting them in a ring.");
				ResetOnRightClick([&] { orbit.gravityOnly = false; });
				if (orbit.gravityOnly)
				{
					DragFloat("Softening", &orbit.softening, 0.005f, 0.01f, 1000.0f);
					SetItemTooltip("Softens the pull near the centre. Higher values give a gentler pull.");
					ResetOnRightClick([&] { orbit.softening = 0.5f; });
				}
				else
				{
					DragFloat3("Axis", &orbit.axis.x, 0.005f, -1.0f, 1.0f);
					SetItemTooltip("Rotation axis of the orbit ring.");
					ResetOnRightClick([&] { orbit.axis = glm::vec3(0.0f, 1.0f, 0.0f); });
					DragFloat("Radius", &orbit.radius, 0.025f, 0.0f, 1000.0f);
					SetItemTooltip("Radius of the orbit ring, in world units.");
					ResetOnRightClick([&] { orbit.radius = 1.0f; });
					DragFloat("Angular Speed", &orbit.angularSpeed, 0.025f, -100.0f, 100.0f);
					SetItemTooltip("How fast particles circle the ring. Negative values reverse the direction.");
					ResetOnRightClick([&] { orbit.angularSpeed = 1.0f; });
				}
				DragFloat("Strength", &orbit.strength, 0.05f, 0.0f, 100.0f);
				SetItemTooltip("How strongly particles are pulled toward the orbit or gravity well.");
				ResetOnRightClick([&] { orbit.strength = 5.0f; });
				if (Button("Remove"))
					removeIndex = i;
				SetItemTooltip("Remove this orbit point.");
				TreePop();
			}
			PopID();
		}
		if (removeIndex < m_orbitPoints.size())
			m_orbitPoints.erase(m_orbitPoints.begin() + removeIndex);
		if (Button("Add Orbit Point"))
			AddOrbitPoint({});
		SetItemTooltip("Add an orbit point at the emitter origin.");
	}

	auto RenderCurve = [&ResetOnRightClick](const char *label, const char *tooltip, ScalarCurve &curve) {
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
		else if (ImGui::BeginPopupContextItem("Curve Options"))
		{
			if (ImGui::Button("Reset"))
				curve = MakeDefaultCurve(1.0f);

			if (ImGui::Combo("Curve Type", &curveType, curveTypeNames, IM_ARRAYSIZE(curveTypeNames)))
				curve.type = static_cast<Curves::CurveType>(curveType);
			ResetOnRightClick([&] { curve.type = Curves::CurveType::Linear; });

			if (ImGui::DragInt("Height", &height, 0.5f, 1))
				storage->SetInt(heightID, height);
			ResetOnRightClick([&] { storage->SetInt(heightID, 150); });

			if (ImGui::DragFloat("Min Value", &minValue, 0.005f))
			{
				minValue = std::min(minValue, maxValue);
				storage->SetFloat(minValueID, minValue);
			}
			ResetOnRightClick([&] { storage->SetFloat(minValueID, 0.0f); });

			if (ImGui::DragFloat("Max Value", &maxValue, 0.005f))
			{
				maxValue = std::max(maxValue, minValue);
				storage->SetFloat(maxValueID, maxValue);
			}
			ResetOnRightClick([&] { storage->SetFloat(maxValueID, 2.0f); });

			if (ImGui::DragFloat("Line Thickness", &lineThickness, 0.05f, 1))
				storage->SetFloat(lineThicknessID, lineThickness);
			ResetOnRightClick([&] { storage->SetFloat(lineThicknessID, 3.0f); });

			ImGui::EndPopup();
		}

		EndChild();
		SetItemTooltip("%s", tooltip);
	};

	if (CollapsingHeader("Over Lifetime", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (TreeNodeEx("Size", ImGuiTreeNodeFlags_DefaultOpen))
		{
			RenderCurve("##Size Over Lifetime", "Multiplier for particle size, from birth (left) to death (right).", m_sizeOverLifetime);
			TreePop();
		}
		if (TreeNodeEx("Velocity", ImGuiTreeNodeFlags_DefaultOpen))
		{
			RenderCurve("##Velocity Over Lifetime", "Multiplier for particle velocity, from birth (left) to death (right).", m_velocityOverLifetime);
			TreePop();
		}
		if (TreeNodeEx("Rotation", ImGuiTreeNodeFlags_DefaultOpen))
		{
			RenderCurve("##Rotation Over Lifetime", "Extra spin added to each particle, from birth (left) to death (right).", m_rotationOverLifetime);
			TreePop();
		}
	}

	if (CollapsingHeader("Color Over Lifetime"))
	{
		TextDisabled("Drag handles to move them. Double-click the bar to add a stop.");
		RenderGradientEditor("##Color Gradient", m_colorOverLifetime);
		if (TreeNodeEx(("Color Stops (" + std::to_string(m_colorOverLifetime.size()) + ")###ColorStops").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			RenderColorStopList(m_colorOverLifetime);
			TreePop();
		}
	}

	if (CollapsingHeader("Rendering"))
	{
		Checkbox("Transparent", &m_transparent);
		SetItemTooltip("Render with alpha blending so semi-transparent particles blend with what is behind them.");
		ResetOnRightClick([&] { m_transparent = true; });

		glm::vec2 billboardOffset = m_billboardOffset;
		if (DragFloat2("Billboard Offset", &billboardOffset.x, 0.005f))
			SetBillboardOffset(billboardOffset);
		SetItemTooltip("Moves each billboard in the camera plane, in world units, relative to its particle.");
		ResetOnRightClick([&] { SetBillboardOffset(glm::vec2(0.0f)); });
		TextDisabled("Shifts each billboard in the camera plane, relative to its particle.");

		GUID newGUID = C_INVALID_GUID;
		std::string newName;
		if (BeginTable("ParticleTexture", 2, ImGuiTableFlags_SizingStretchProp))
		{
			TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 120.0f);
			TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);
			if (ResourceSlot("Texture", m_texture.GetGUID(), m_textureDisplayName, newGUID, newName, StaticResourceType::Texture2D))
				SetTextureGUID(newGUID);
			newGUID = C_INVALID_GUID;
			newName.clear();
			if (ResourceSlot("Vertex Shader", m_vs.GetGUID(), m_vsDisplayName, newGUID, newName,
				StaticResourceType::Shader, ShaderResource::ShaderType::Vertex))
				SetVSGUID(newGUID);
			newGUID = C_INVALID_GUID;
			newName.clear();
			if (ResourceSlot("Geometry Shader", m_gs.GetGUID(), m_gsDisplayName, newGUID, newName,
				StaticResourceType::Shader, ShaderResource::ShaderType::Geometry))
				SetGSGUID(newGUID);
			newGUID = C_INVALID_GUID;
			newName.clear();
			if (ResourceSlot("Fragment Shader", m_fs.GetGUID(), m_fsDisplayName, newGUID, newName,
				StaticResourceType::Shader, ShaderResource::ShaderType::Fragment))
				SetFSGUID(newGUID);
			EndTable();
		}
	}

	if (CollapsingHeader("Debug"))
	{
		Checkbox("Debug Draw", &m_debugDraw);
		SetItemTooltip("Show the emission shape, direction cone and orbit rings in the viewport.");
		ResetOnRightClick([&] { m_debugDraw = false; });
		TextDisabled("Shows the emission shape, fixed direction cone and orbit rings in the viewport.");
	}

	PopItemWidth();
	PopID();
#endif
}


void ParticleEmitterComponent::GenerateParticleMesh()
{
	ZoneScoped;

	struct ParticleVertex
	{
		glm::vec3 position;
		glm::vec4 color;
		glm::vec2 size;
		float rotation;
		glm::vec2 billboardOffset;
	};
	
	std::vector<ParticleVertex> vertices;
	std::vector<std::uint32_t> indices;
	vertices.reserve(m_particles.size());
	indices.reserve(m_particles.size());

	glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
	glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());

	std::uint32_t index = 0;
	for (const Particle &p : m_particles)
	{
		glm::vec3 pos = p.position;
		glm::vec2 size = p.size;
		float rot = p.rotation;

		if (m_simulationSpace == SimulationSpace::World)
		{
			Transform &transform = GetTransform();
			glm::mat4 worldMatrix = transform.GetMatrix(Transform::Space::World);
			glm::mat4 invWorldMatrix = glm::inverse(worldMatrix);

			// Undo the world transform to get the local position for rendering
			pos = glm::vec3(invWorldMatrix * glm::vec4(pos, 1.0f));
		}

		// Conservative bounds: a square of the larger dimension covers any rotation of the quad, plus the billboard offset
		const float extent = std::max(size.x, size.y) * 0.5f + glm::length(m_billboardOffset);
		min = glm::min(min, pos - glm::vec3(extent));
		max = glm::max(max, pos + glm::vec3(extent));

		vertices.emplace_back(
			pos,
			p.color,
			size,
			rot,
			m_billboardOffset
		);
		indices.emplace_back(index++);
	}

	const std::span<const std::byte> bytes = std::as_bytes(std::span<const ParticleVertex>(vertices));

	m_particleMesh = std::make_shared<const Common::DynamicMeshData>(Common::DynamicMeshData{
		.vertexStride = sizeof(ParticleVertex),
		.attributes = {
			{.location = 0, .format = Common::VertexFormat::Float3, .offset = offsetof(ParticleVertex, position)},
			{.location = 1, .format = Common::VertexFormat::Float4, .offset = offsetof(ParticleVertex, color)},
			{.location = 2, .format = Common::VertexFormat::Float2, .offset = offsetof(ParticleVertex, size)},
			{.location = 3, .format = Common::VertexFormat::Float1, .offset = offsetof(ParticleVertex, rotation)},
			{.location = 4, .format = Common::VertexFormat::Float2, .offset = offsetof(ParticleVertex, billboardOffset)}
		},
		.vertexData = { bytes.begin(), bytes.end() },
		.indices = std::move(indices),
		.topology = Common::MeshTopology::Points
	}); 
	
	UpdateBounds(min, max);
}
