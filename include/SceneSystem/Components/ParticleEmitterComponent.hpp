#pragma once
#include <SceneSystem/Component.hpp>
#include <Math/Curves.hpp>
#include <Resource/StaticResourceHandle.hpp>
#include <Resource/Types/Texture2DResource.hpp>
#include <Resource/Types/ShaderResource.hpp>
#include <SceneSystem/Bounds/DefaultNodeBounds.hpp>

#include <glm/glm.hpp>
#include <algorithm>
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

		enum class DirectionMode
		{
			Shape,
			Fixed,
			TowardsPoint,		///< Every particle is aimed at the emitter's target point
			RandomPointInShape	///< Every particle is aimed at a random point inside the emission shape
		};

		/// @brief How a particle picks its tile when the texture is a particle sheet
		enum class TileSelection
		{
			Fixed,			///< Every particle uses the tile index
			Random,			///< Each particle picks a random tile from the tile range when it spawns
			OverLifetime	///< Each particle steps through the tile range from birth to death
		};

		struct FloatRange
		{
			float min = 1.0f;
			float max = 1.0f;
		};

		/// @brief Inclusive range of tile indices, row-major from the top-left tile
		struct TileRange
		{
			uint32_t min = 0;
			uint32_t max = 0;
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

		/// @brief A color key on the normalized lifetime axis [0, 1]. Colors are multiplied with the start color.
		struct ColorStop
		{
			float time = 0.0f;
			glm::vec4 color = glm::vec4(1.0f);
		};

		/// @brief A point (in emitter-local space) that pulls particles into an orbit ring around its axis.
		struct OrbitPoint
		{
			glm::vec3 position = glm::vec3(0.0f);
			glm::vec3 axis = glm::vec3(0.0f, 1.0f, 0.0f);
			float radius = 1.0f;
			float angularSpeed = 1.0f;	///< Radians per second
			float strength = 5.0f;		///< Pull and steering gain
			bool gravityOnly = false;	///< Attract with inverse-square gravity instead of forming an orbit ring
			float softening = 0.5f;		///< Gravity-only: distance below which the pull is softened
		};

		struct Particle
		{
			glm::vec3 position = glm::vec3(0.0f);
			glm::vec3 velocity = glm::vec3(0.0f);
			glm::vec4 color = glm::vec4(1.0f);
			glm::vec4 baseColor = glm::vec4(1.0f);
			glm::vec2 size = glm::vec2(1.0f);		///< Width and height, after size over lifetime
			glm::vec2 baseSize = glm::vec2(1.0f);	///< Width and height at spawn
			float rotation = 0.0f;
			float angularVelocity = 0.0f;
			float age = 0.0f;
			float lifetime = 1.0f;
			float tileRoll = 0.0f;	///< Random in [0, 1], chosen at spawn to pick a tile from the tile range
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
		[[nodiscard]] float GetTimeScale() const { return m_timeScale; }
		[[nodiscard]] float GetUpdateRate() const { return m_updateRate; }
		[[nodiscard]] uint32_t GetMaxParticles() const { return m_maxParticles; }
		[[nodiscard]] float GetEmissionRate() const { return m_emissionRate; }
		[[nodiscard]] float GetEmissionRateOverDistance() const { return m_emissionRateOverDistance; }
		[[nodiscard]] bool GetRandomizeEmissionRate() const { return m_randomizeEmissionRate; }
		[[nodiscard]] FloatRange GetEmissionRateRange() const { return m_emissionRateRange; }
		[[nodiscard]] float GetInheritVelocity() const { return m_inheritVelocity; }

		void SetDuration(float duration);
		void SetTimeScale(float timeScale);
		void SetUpdateRate(float updatesPerSecond);
		void SetLooping(bool looping);
		void SetPlayOnAwake(bool playOnAwake);
		void SetStartDelay(float startDelay);
		void SetMaxParticles(uint32_t maxParticles);
		void SetEmissionRate(float particlesPerSecond);
		void SetEmissionRateOverDistance(float particlesPerUnit);
		void SetRandomizeEmissionRate(bool randomize);
		void SetEmissionRateRange(const FloatRange &range);
		void SetInheritVelocity(float inheritVelocity);
		void SetLifetimeRange(const FloatRange &range);
		void SetSpeedRange(const FloatRange &range);
		void SetWidthRange(const FloatRange &range);
		void SetHeightRange(const FloatRange &range);
		void SetRotationRange(const FloatRange &range);
		void SetBillboardOffset(const glm::vec2 &offset);
		void SetAngularVelocityRange(const FloatRange &range);
		void SetStartColorRange(const ColorRange &range);
		void SetStartColorMode(StartColorMode mode);
		void SetGravityModifier(float gravityModifier);
		void SetEmissionShape(EmissionShape shape);
		void SetSimulationSpace(SimulationSpace space);
		void SetRandomSeed(uint32_t seed);
		[[nodiscard]] uint32_t GetRandomSeed() const { return m_randomSeed; }
		void SetRandomSeedEachPlay(bool enabled) { m_randomSeedEachPlay = enabled; }
		[[nodiscard]] bool GetRandomSeedEachPlay() const { return m_randomSeedEachPlay; }

		void SetDirectionMode(DirectionMode mode) { m_directionMode = mode; }
		[[nodiscard]] DirectionMode GetDirectionMode() const { return m_directionMode; }
		void SetDirection(const glm::vec3 &direction) { m_direction = direction; }
		[[nodiscard]] const glm::vec3 &GetDirection() const { return m_direction; }
		void SetDirectionSpread(float angleDegrees) { m_directionSpread = std::clamp(angleDegrees, 0.0f, 180.0f); }
		[[nodiscard]] float GetDirectionSpread() const { return m_directionSpread; }
		void SetTargetPoint(const glm::vec3 &point) { m_targetPoint = point; }
		[[nodiscard]] const glm::vec3 &GetTargetPoint() const { return m_targetPoint; }

		void SetAcceleration(const glm::vec3 &acceleration) { m_acceleration = acceleration; }
		[[nodiscard]] const glm::vec3 &GetAcceleration() const { return m_acceleration; }
		void SetTurbulenceStrength(float strength) { m_turbulenceStrength = std::max(0.0f, strength); }
		[[nodiscard]] float GetTurbulenceStrength() const { return m_turbulenceStrength; }
		void SetTurbulenceFrequency(float frequency) { m_turbulenceFrequency = std::max(0.0f, frequency); }
		[[nodiscard]] float GetTurbulenceFrequency() const { return m_turbulenceFrequency; }

		/// @brief Replaces the color gradient. Stops are sorted by time; an empty list restores a single white stop.
		void SetColorOverLifetime(std::vector<ColorStop> stops);
		[[nodiscard]] const std::vector<ColorStop> &GetColorOverLifetime() const { return m_colorOverLifetime; }
		[[nodiscard]] std::vector<ColorStop> &GetColorOverLifetime() { return m_colorOverLifetime; }
		void SortColorOverLifetime();

		void AddOrbitPoint(const OrbitPoint &point) { m_orbitPoints.push_back(point); }
		void ClearOrbitPoints() { m_orbitPoints.clear(); }
		[[nodiscard]] const std::vector<OrbitPoint> &GetOrbitPoints() const { return m_orbitPoints; }
		[[nodiscard]] std::vector<OrbitPoint> &GetOrbitPoints() { return m_orbitPoints; }

		[[nodiscard]] FloatRange GetLifetimeRange() const { return m_lifetimeRange; }
		[[nodiscard]] FloatRange GetSpeedRange() const { return m_speedRange; }
		[[nodiscard]] FloatRange GetWidthRange() const { return m_widthRange; }
		[[nodiscard]] FloatRange GetHeightRange() const { return m_heightRange; }
		[[nodiscard]] FloatRange GetRotationRange() const { return m_rotationRange; }
		[[nodiscard]] glm::vec2 GetBillboardOffset() const { return m_billboardOffset; }
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

		static constexpr uint32_t MaxSheetDimension = 256;
		void SetParticleSheet(bool particleSheet) { m_particleSheet = particleSheet; }
		[[nodiscard]] bool GetParticleSheet() const { return m_particleSheet; }
		void SetSheetColumns(uint32_t columns) { m_sheetColumns = std::clamp(columns, 1u, MaxSheetDimension); }
		[[nodiscard]] uint32_t GetSheetColumns() const { return m_sheetColumns; }
		void SetSheetRows(uint32_t rows) { m_sheetRows = std::clamp(rows, 1u, MaxSheetDimension); }
		[[nodiscard]] uint32_t GetSheetRows() const { return m_sheetRows; }
		void SetTileSelection(TileSelection selection) { m_tileSelection = selection; }
		[[nodiscard]] TileSelection GetTileSelection() const { return m_tileSelection; }
		void SetTileIndex(uint32_t index) { m_tileIndex = index; }
		[[nodiscard]] uint32_t GetTileIndex() const { return m_tileIndex; }
		void SetTileRange(const TileRange &range) { m_tileRange = range; }
		[[nodiscard]] TileRange GetTileRange() const { return m_tileRange; }

		void SetVSGUID(GUID guid);
		void SetGSGUID(GUID guid);
		void SetFSGUID(GUID guid);

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
		glm::vec4 EvaluateColorOverLifetime(float normalizedTime) const;
		glm::vec4 TileUVRect(const Particle &p_particle) const;
		float RandomFloat(float min, float max);
		void RollEmissionRate();
		glm::vec4 RandomStartColor();
		glm::vec3 RandomUnitSphere();
		glm::vec3 RandomDirectionForEmission();
		glm::vec3 RandomDirectionInCone(const glm::vec3 &axis, float spreadDegrees);
		glm::vec3 RandomPointInEmissionVolume();
		void SpawnParticle();
		void UpdateEmitter(float deltaTime);
		void UpdateParticles(float deltaTime);
		glm::vec3 UpdateOwnerMotion(float deltaTime);
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
		glm::vec2 m_billboardOffset = glm::vec2(0.0f);
		bool m_debugDraw = false;
		bool m_uiSelected = false;
		float m_duration = 5.0f;
		float m_startDelay = 0.0f;
		float m_timeScale = 1.0f;
		float m_updateRate = 0.0f;
		float m_updateAccumulator = 0.0f;
		float m_delayElapsed = 0.0f;
		float m_systemTime = 0.0f;
		float m_emissionAccumulator = 0.0f;
		float m_emissionRate = 10.0f;
		float m_emissionRateOverDistance = 0.0f;
		bool m_randomizeEmissionRate = false;
		FloatRange m_emissionRateRange = { 5.0f, 15.0f };
		float m_activeEmissionRate = 10.0f;
		float m_inheritVelocity = 0.0f;
		float m_gravityModifier = 0.0f;
		float m_shapeRadius = 1.0f;
		float m_coneAngle = 25.0f;
		float m_directionSpread = 0.0f;
		float m_turbulenceStrength = 0.0f;
		float m_turbulenceFrequency = 1.0f;
		float m_turbulenceTime = 0.0f;
		glm::vec3 m_shapeBoxSize = glm::vec3(1.0f);
		glm::vec3 m_direction = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 m_targetPoint = glm::vec3(0.0f, 5.0f, 0.0f);
		glm::vec3 m_acceleration = glm::vec3(0.0f);
		glm::vec3 m_emitterVelocity = glm::vec3(0.0f);
		glm::vec3 m_lastOwnerPosition = glm::vec3(0.0f);
		bool m_hasLastOwnerPosition = false;
		uint32_t m_maxParticles = 1000;
		uint32_t m_randomSeed = 0xD20B1E7u;
		bool m_randomSeedEachPlay = false;

		EmissionShape m_emissionShape = EmissionShape::Point;
		SimulationSpace m_simulationSpace = SimulationSpace::World;
		StartColorMode m_startColorMode = StartColorMode::Lerp;
		DirectionMode m_directionMode = DirectionMode::Shape;
		FloatRange m_lifetimeRange{ 1.0f, 2.0f };
		FloatRange m_speedRange{ 5.0f, 5.0f };
		FloatRange m_widthRange{ 0.1f, 0.1f };
		FloatRange m_heightRange{ 0.1f, 0.1f };
		FloatRange m_rotationRange{ 0.0f, 360.0f };
		FloatRange m_angularVelocityRange{ 0.0f, 0.0f };
		ColorRange m_startColorRange{};

		ScalarCurve m_sizeOverLifetime = MakeDefaultCurve(1.0f);
		ScalarCurve m_velocityOverLifetime = MakeDefaultCurve(1.0f);
		ScalarCurve m_rotationOverLifetime = MakeDefaultCurve(0.0f);
		std::vector<ColorStop> m_colorOverLifetime{ { 0.0f, glm::vec4(1.0f) }, { 1.0f, glm::vec4(1.0f) } };

		std::vector<Burst> m_bursts;
		std::vector<OrbitPoint> m_orbitPoints;
		std::vector<Particle> m_particles;
		std::mt19937 m_random{ m_randomSeed };


		StaticResourceHandle<Texture2DResource> m_texture{};
		std::string m_textureDisplayName = "None";
		bool m_particleSheet = false;
		uint32_t m_sheetColumns = 1;
		uint32_t m_sheetRows = 1;
		TileSelection m_tileSelection = TileSelection::Fixed;
		uint32_t m_tileIndex = 0;
		TileRange m_tileRange{ 0, 0 };

		StaticResourceHandle<ShaderResource> m_vs{};
		std::string m_vsDisplayName = "None";

		StaticResourceHandle<ShaderResource> m_gs{};
		std::string m_gsDisplayName = "None";

		StaticResourceHandle<ShaderResource> m_fs{};
		std::string m_fsDisplayName = "None";

		std::shared_ptr<const Common::DynamicMeshData> m_particleMesh{};
	};
}
