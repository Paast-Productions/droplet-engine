#pragma once
#include <SceneSystem/Component.hpp>
#include <Math/Curves.hpp>
#include <Resource/StaticResourceHandle.hpp>
#include <Resource/Types/Texture2DResource.hpp>
#include <Resource/Types/ShaderResource.hpp>
#include <SceneSystem/Bounds/DefaultNodeBounds.hpp>

#include <glm/glm.hpp>
#include <random>
#include <vector>

namespace Droplet::Scene
{
	class ParticleEmitterComponent : public Component
	{
	public:
		enum class EmissionShape
		{
			Point,
			Sphere,
			Hemisphere,
			Box,
			Cone,
			Circle
		};

		enum class SimulationSpace
		{
			Local,
			World
		};

		enum class StartColorMode
		{
			RandomPerComponent,
			Lerp,
			SphericalLerp
		};

		struct FloatRange
		{
			float min = 1.0f;
			float max = 1.0f;
		};

		struct ColorRange
		{
			glm::vec4 min = glm::vec4(1.0f);
			glm::vec4 max = glm::vec4(1.0f);
		};

		struct Burst
		{
			float time = 0.0f;
			uint32_t count = 30;
			bool triggered = false;
		};

		struct Particle
		{
			glm::vec3 position = glm::vec3(0.0f);
			glm::vec3 velocity = glm::vec3(0.0f);
			glm::vec4 color = glm::vec4(1.0f);
			float size = 1.0f;
			float baseSize = 1.0f;
			float rotation = 0.0f;
			float angularVelocity = 0.0f;
			float age = 0.0f;
			float lifetime = 1.0f;
		};

		using ScalarCurve = Droplet::Math::Curves::Curve<glm::vec2>;

		ParticleEmitterComponent();

		[[nodiscard]] std::string_view GetTypeName() const override { return "ParticleEmitterComponent"; }
		[[nodiscard]] bool HasBoundsOverride() const override { return true; }
		[[nodiscard]] const NodeBounds *GetBounds() const override { return &m_bounds; }

		void Play();
		void Pause();
		void Stop(bool clearParticles = true);
		void Emit(uint32_t count);

		void SetPlaying(bool playing);
		[[nodiscard]] bool IsPlaying() const { return m_playing; }
		[[nodiscard]] bool IsPaused() const { return m_paused; }

		[[nodiscard]] const std::vector<Particle> &GetParticles() const { return m_particles; }

		[[nodiscard]] float GetDuration() const { return m_duration; }
		[[nodiscard]] bool GetLooping() const { return m_looping; }
		[[nodiscard]] bool GetPlayOnAwake() const { return m_playOnAwake; }
		[[nodiscard]] float GetStartDelay() const { return m_startDelay; }
		[[nodiscard]] uint32_t GetMaxParticles() const { return m_maxParticles; }
		[[nodiscard]] float GetEmissionRate() const { return m_emissionRate; }

		void SetDuration(float duration);
		void SetLooping(bool looping);
		void SetPlayOnAwake(bool playOnAwake);
		void SetStartDelay(float startDelay);
		void SetMaxParticles(uint32_t maxParticles);
		void SetEmissionRate(float particlesPerSecond);
		void SetLifetimeRange(const FloatRange &range);
		void SetSpeedRange(const FloatRange &range);
		void SetSizeRange(const FloatRange &range);
		void SetRotationRange(const FloatRange &range);
		void SetAngularVelocityRange(const FloatRange &range);
		void SetStartColorRange(const ColorRange &range);
		void SetStartColorMode(StartColorMode mode);
		void SetGravityModifier(float gravityModifier);
		void SetEmissionShape(EmissionShape shape);
		void SetSimulationSpace(SimulationSpace space);

		[[nodiscard]] FloatRange GetLifetimeRange() const { return m_lifetimeRange; }
		[[nodiscard]] FloatRange GetSpeedRange() const { return m_speedRange; }
		[[nodiscard]] FloatRange GetSizeRange() const { return m_sizeRange; }
		[[nodiscard]] FloatRange GetRotationRange() const { return m_rotationRange; }
		[[nodiscard]] FloatRange GetAngularVelocityRange() const { return m_angularVelocityRange; }
		[[nodiscard]] ColorRange GetStartColorRange() const { return m_startColorRange; }
		[[nodiscard]] StartColorMode GetStartColorMode() const { return m_startColorMode; }

		void SetShapeRadius(float radius);
		void SetShapeBoxSize(const glm::vec3 &size);
		void SetConeAngle(float angleDegrees);

		void SetDebugDraw(bool debugDraw) { m_debugDraw = debugDraw; }
		[[nodiscard]] bool GetDebugDraw() const { return m_debugDraw; }

		void AddBurst(float time, uint32_t count);
		void ClearBursts();
		[[nodiscard]] const std::vector<Burst> &GetBursts() const { return m_bursts; }

		ScalarCurve &GetSizeOverLifetime() { return m_sizeOverLifetime; }
		ScalarCurve &GetVelocityMultiplierOverLifetime() { return m_velocityOverLifetime; }
		ScalarCurve &GetRotationOverLifetime() { return m_rotationOverLifetime; }
		const ScalarCurve &GetSizeOverLifetime() const { return m_sizeOverLifetime; }
		const ScalarCurve &GetVelocityMultiplierOverLifetime() const { return m_velocityOverLifetime; }
		const ScalarCurve &GetRotationOverLifetime() const { return m_rotationOverLifetime; }

		void SetTextureGUID(GUID guid);
		[[nodiscard]] GUID GetTextureGUID() const { return m_texture.GetGUID(); }

		void GenerateParticleMesh();

	protected:
		void StartImpl() override;
		void UpdateImpl() override;
		void EditorUpdateImpl() override;
		void QueueRenderImpl() override;
		void RenderUIImpl() override;
		[[nodiscard]] Droplet::Common::RenderData GetRenderDataImpl() override;
		nlohmann::json SerializeImpl() override;
		void DeserializeImpl(nlohmann::json p_compJson) override;

	private:
		static ScalarCurve MakeDefaultCurve(float value);

		float EvaluateCurve(const ScalarCurve &curve, float normalizedTime) const;
		float RandomFloat(float min, float max);
		glm::vec4 RandomStartColor();
		glm::vec3 RandomUnitSphere();
		glm::vec3 RandomDirectionForEmission();
		void SpawnParticle();
		void UpdateEmitter(float deltaTime);
		void UpdateParticles(float deltaTime);
		void UpdateBurstEmission();
		void ResetBurstTriggers();
		void DrawDebugParticles();
		void DrawDebugEmitter() const;
		void SerializeCurve(nlohmann::json &json, const char *key, const ScalarCurve &curve) const;
		void DeserializeCurve(const nlohmann::json &json, const char *key, ScalarCurve &curve);

		void UpdateBounds(glm::vec3 p_min, glm::vec3 p_max);

		DefaultNodeBounds m_bounds{ glm::vec3(0.0f), glm::vec3(1.0f) };

		bool m_playing = false;
		bool m_paused = false;
		bool m_playOnAwake = true;
		bool m_looping = true;
		bool m_transparent = true;
		bool m_debugDraw = false;
		bool m_uiSelected = false;
		float m_duration = 5.0f;
		float m_startDelay = 0.0f;
		float m_delayElapsed = 0.0f;
		float m_systemTime = 0.0f;
		float m_emissionAccumulator = 0.0f;
		float m_emissionRate = 10.0f;
		float m_gravityModifier = 0.0f;
		float m_shapeRadius = 1.0f;
		float m_coneAngle = 25.0f;
		glm::vec3 m_shapeBoxSize = glm::vec3(1.0f);
		uint32_t m_maxParticles = 1000;

		EmissionShape m_emissionShape = EmissionShape::Point;
		SimulationSpace m_simulationSpace = SimulationSpace::World;
		StartColorMode m_startColorMode = StartColorMode::Lerp;
		FloatRange m_lifetimeRange{ 1.0f, 2.0f };
		FloatRange m_speedRange{ 5.0f, 5.0f };
		FloatRange m_sizeRange{ 0.1f, 0.1f };
		FloatRange m_rotationRange{ 0.0f, 360.0f };
		FloatRange m_angularVelocityRange{ 0.0f, 0.0f };
		ColorRange m_startColorRange{};

		ScalarCurve m_sizeOverLifetime = MakeDefaultCurve(1.0f);
		ScalarCurve m_velocityOverLifetime = MakeDefaultCurve(1.0f);
		ScalarCurve m_rotationOverLifetime = MakeDefaultCurve(0.0f);

		std::vector<Burst> m_bursts;
		std::vector<Particle> m_particles;
		std::mt19937 m_random{ 0xD20B1E7u };

		StaticResourceHandle<Texture2DResource> m_texture{};
		std::string m_textureDisplayName = "None";

		StaticResourceHandle<ShaderResource> m_vs{};
		std::string m_vsDisplayName = "None";

		StaticResourceHandle<ShaderResource> m_gs{};
		std::string m_gsDisplayName = "None";

		StaticResourceHandle<ShaderResource> m_fs{};
		std::string m_fsDisplayName = "None";

		std::shared_ptr<const Common::DynamicMeshData> m_particleMesh{};
	};
}
