#pragma once

#include <Jolt/Jolt.h>
#include <unordered_map>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <resource/Types/MeshResource.hpp>
#include <resource/GuidUtils.hpp>

/// @brief Class caching loaded mesh colliders
class MeshColliderManager
{
public:
    /// @brief Copy assignment operator deleting itself
    /// @param MeshColliderManager
    MeshColliderManager &operator=(const MeshColliderManager &) = delete;
    
    [[nodiscard]] static MeshColliderManager &Get()
    {
        static MeshColliderManager instance{};
        return instance;
    }
    
	/// @brief Get mesh shape settings
	///
	/// If the mesh shape settings isn't cached, the mesh shape settings will be created and stored
	/// @param p_guid The GUID of the mesh
    JPH::MeshShapeSettings GetMeshSettings(const Droplet::GUID p_guid);
	
	void UnloadMeshSetting(const Droplet::GUID p_guid);
    
private:
	MeshColliderManager() = default;
	~MeshColliderManager() = default;
	
	struct MeshColliderCache
	{
		JPH::MeshShapeSettings meshShapeSettings{};
		
		std::uint32_t GetRefCount() const { return refCount; }
		void AddRef() { refCount++; }
		void Release() { refCount--; }
		
	private:
		std::uint32_t refCount{0};
		
	};
    
	/// @brief Helper function creating mesh collider shape settings
	/// @param p_mesh The mesh resource to make a collider of
	static JPH::MeshShapeSettings CreateMeshShapeSettings(const Droplet::MeshResource *p_mesh);

    std::unordered_map<Droplet::GUID, MeshColliderCache> m_shapeSettings{};
    
};
