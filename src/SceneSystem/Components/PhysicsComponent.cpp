#include "PhysicsComponent.hpp"

#include <SceneSystem/Node.hpp>
#include <SceneSystem/Scene.hpp>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <ImGui/imgui.h>
#include <SceneSystem/Components/MeshComponent.hpp>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>

using namespace Droplet::Scene;

PhysicsComponent::~PhysicsComponent()
{
	RemoveCollider();
}

void PhysicsComponent::Initialize(Physics *p_physics, Transform *p_transform)
{
	m_physics = p_physics;
	m_transform = p_transform;
}

void PhysicsComponent::SetBoxCollider(const JPH::Vec3 p_halfExtents, const JPH::Vec3 &p_offset)
{
	if (m_shapeType != ShapeType::None)
	{
		RemoveCollider();
	}

	JPH::Ref<JPH::BoxShapeSettings> boxSettings = new JPH::BoxShapeSettings(p_halfExtents);
	CreateCollider(boxSettings, p_offset);
	m_shapeType = ShapeType::Box;
}

void PhysicsComponent::SetBoxCollider(const glm::vec3 p_halfExtents, const JPH::Vec3 &p_offset)
{
	SetBoxCollider(JPH::Vec3(p_halfExtents.x, p_halfExtents.y, p_halfExtents.z), p_offset);
}

void PhysicsComponent::SetSphereCollider(const float p_radius, const JPH::Vec3 &p_offset)
{
	if (m_shapeType != ShapeType::None)
	{
		RemoveCollider();
	}

	JPH::Ref<JPH::SphereShapeSettings> sphereSettings = new JPH::SphereShapeSettings(p_radius);
	CreateCollider(sphereSettings, p_offset);
	m_shapeType = ShapeType::Sphere;
}

void PhysicsComponent::SetCapsuleCollider(const float p_HalfHeightOfCylinder, const float p_radius, const JPH::Vec3 &p_offset)
{
	if (m_shapeType != ShapeType::None)
	{
		RemoveCollider();
	}

	JPH::Ref<JPH::CapsuleShapeSettings> capsuleSettings = new JPH::CapsuleShapeSettings(p_HalfHeightOfCylinder, p_radius);
	CreateCollider(capsuleSettings, p_offset);
	m_shapeType = ShapeType::Capsule;
}

void PhysicsComponent::SetMeshCollider(const Droplet::MeshResource *p_mesh, const JPH::Vec3 &p_offset)
{
	if (m_shapeType != ShapeType::None || p_mesh == nullptr)
	{
		RemoveCollider();
	}

	// Make a Jolt array for vertices
	const std::size_t C_BYTES_PER_VERTEX = p_mesh->GetVertexByteSize();
	const std::size_t C_VERTEX_COUNT = p_mesh->GetVertexData().size() / C_BYTES_PER_VERTEX;
	
	JPH::VertexList joltVertices{};
	joltVertices.reserve(C_VERTEX_COUNT);
	
	std::vector<std::byte> vertexData = p_mesh->GetVertexData();
	for (std::size_t i = 0; i < C_VERTEX_COUNT; i++)
	{
		JPH::Float3 pos;
		std::memcpy(&pos, vertexData.data() + i * C_BYTES_PER_VERTEX, sizeof(JPH::Float3));
		
		joltVertices.emplace_back(pos);
	}

	// Make a Jolt array for triangles based of indices
	JPH::IndexedTriangleList triangles;
	std::vector<std::uint32_t> indexData = p_mesh->GetIndexData();
	std::size_t nrOfIndices = indexData.size();
	for (std::size_t i = 0; i < nrOfIndices; i += 3)
	{
		triangles.emplace_back(
			indexData[i],
			indexData[i + 1],
			indexData[i + 2]
		);
	}

	JPH::Ref<JPH::MeshShapeSettings> meshSettings = new JPH::MeshShapeSettings(std::move(joltVertices), std::move(triangles));
	CreateCollider(meshSettings, p_offset);
	m_shapeType = ShapeType::Mesh;
}

void PhysicsComponent::SetConvexHullCollider(const Droplet::MeshResource *p_mesh, float p_edgeRadius, const JPH::Vec3 &p_offset)
{
	if (m_shapeType != ShapeType::None || p_mesh == nullptr)
	{
		RemoveCollider();
	}
	
	// Make a Jolt array for vertices
	const std::size_t C_BYTES_PER_VERTEX = p_mesh->GetVertexByteSize();
	const std::size_t C_VERTEX_COUNT = p_mesh->GetVertexData().size() / C_BYTES_PER_VERTEX;
	
	JPH::Array<JPH::Vec3> points;
	points.reserve(C_VERTEX_COUNT);
	
	std::vector<std::byte> vertexData = p_mesh->GetVertexData();
	for (size_t i = 0; i < C_VERTEX_COUNT; ++i)
	{
		JPH::Vec3 pos;
		std::memcpy(&pos, vertexData.data() + i * C_BYTES_PER_VERTEX, sizeof(JPH::Vec3));
		
		points.emplace_back(pos);
	}
	
	JPH::Ref<JPH::ConvexHullShapeSettings> convexSettings = nullptr;
	if (p_edgeRadius < 0.0f)
	{
		convexSettings = new JPH::ConvexHullShapeSettings(points);
	}
	else
	{
		convexSettings = new JPH::ConvexHullShapeSettings(points, p_edgeRadius);
	}
	CreateCollider(convexSettings, p_offset);
	m_shapeType = ShapeType::Mesh;
}

void PhysicsComponent::SetMotionType(const JPH::EMotionType p_motionType)
{
	m_physics->GetBodyInterface()->SetMotionType(m_bodyID, p_motionType, JPH::EActivation::Activate);
	m_motion = p_motionType;
}

void PhysicsComponent::SetObjectLayer(const Layers p_objectLayer)
{
	m_physics->GetBodyInterface()->SetObjectLayer(m_bodyID, p_objectLayer);
	m_layer = p_objectLayer;
}

void PhysicsComponent::SetActive(bool p_active)
{
	if (p_active)
	{
		m_physics->GetBodyInterface()->ActivateBody(m_bodyID);
	}
	else
	{
		m_physics->GetBodyInterface()->DeactivateBody(m_bodyID);
	}
}

void PhysicsComponent::RemoveCollider()
{
	if (m_shapeType != ShapeType::None)
	{
		JPH::BodyInterface *bodyInterface = m_physics->GetBodyInterface();
		bodyInterface->RemoveBody(m_bodyID);
		bodyInterface->DestroyBody(m_bodyID);
		m_shapeType = ShapeType::None;
	}
}

void PhysicsComponent::StartImpl()
{
	m_physics = GetOwner()->GetScene()->GetPhysics();
	if (m_physics == nullptr)
	{
		GetOwner()->GetScene()->InitializePhysics();
		m_physics = GetOwner()->GetScene()->GetPhysics();
	}
	m_transform = &GetOwner()->GetTransform();
}

void PhysicsComponent::UpdateImpl()
{
	if (m_transform->HasBeenChanged())
	{
		PushTransformToPhysics();
		m_physics->GetBodyInterface()->SetLinearVelocity(m_bodyID, JPH::Vec3(0.0f, 0.0f, 0.0f));
	}
}

void PhysicsComponent::LateUpdateImpl()
{
	PullTransformFromPhysics();
}

void PhysicsComponent::RenderUIImpl()
{
	if (m_shapeType != ShapeType::None)
	{
		// Display the current collider shape
		std::string shapeType = "Shape type: ";
		switch (m_shapeType)
		{
		case ShapeType::Box:
			shapeType += "Box";
			break;
		case ShapeType::Sphere:
			shapeType += "Sphere";
			break;
		case ShapeType::Capsule:
			shapeType += "Capsule";
			break;
		case ShapeType::Mesh:
			shapeType += "Mesh";
			break;
		case ShapeType::ConvexHull:
			shapeType += "Convex Hull";
			break;
		}
		
		ImGui::Text(shapeType.c_str());
		
		// Display the current motion type 
		std::string motionText = "Motion type: ";
		switch (m_motion)
		{
		case JPH::EMotionType::Static:
			motionText += "Static";
			break;
		case JPH::EMotionType::Kinematic:
			motionText += "Kinematic";
			break;
		case JPH::EMotionType::Dynamic:
			motionText += "Dynamic";
			break;
		}
		ImGui::Text(motionText.c_str());
	
		// Display the current layer type
		std::string layerText = "Layer type: ";
		switch (m_layer)
		{
		case NON_MOVING: 
			layerText += "Non-moving";
			break;
		case MOVING: 
			layerText += "Moving";
			break;
		case NUM_LAYERS: 
			layerText += "NumLayers";
			break;
		}
		ImGui::Text(layerText.c_str());

		ImGui::NewLine();
		
		// Display and apply MotionType and ObjectLayer
		const char* motionTypes[] = { "Static", "Kinematic", "Dynamic" };
		const char* objectLayers[] = { "Non-moving", "Moving", "NumLayers" };
		static int motionSelectedIdx = static_cast<int>(m_motion);
		static int layerSelectedIdx = m_layer;
		
		const char* motionPreview = motionTypes[motionSelectedIdx];
		const char* layerPreview = objectLayers[layerSelectedIdx];
		
		if (ImGui::BeginCombo("Motion type", motionPreview, 0))
		{
			for (int n = 0; n < IM_COUNTOF(motionTypes); n++)
			{
				const bool isSelected = (motionSelectedIdx == n);
				if (ImGui::Selectable(motionTypes[n], isSelected))
				{
					motionSelectedIdx = n;
				}
				
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		
		if (ImGui::BeginCombo("Object layer", layerPreview, 0))
		{
			for (int n = 0; n < IM_COUNTOF(objectLayers); n++)
			{
				const bool isSelected = (layerSelectedIdx == n);
				if (ImGui::Selectable(objectLayers[n], isSelected))
				{
					layerSelectedIdx = n;
				}
				
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		
		JPH::EMotionType motion = m_motion;
		switch (static_cast<JPH::EMotionType>(motionSelectedIdx))
		{
		case JPH::EMotionType::Static: 
			motion = JPH::EMotionType::Static;
			break;
		case JPH::EMotionType::Kinematic: 
			motion = JPH::EMotionType::Kinematic;
			break;
		case JPH::EMotionType::Dynamic: 
			motion = JPH::EMotionType::Dynamic;
			break;
		}
		
		Layers layer = m_layer;
		switch (layerSelectedIdx)
		{
		case NON_MOVING: 
			layer = NON_MOVING;
			break;
		case MOVING: 
			layer = MOVING;
			break;
		case NUM_LAYERS: 
			layer = NUM_LAYERS;
			break;
		}
		
		if (ImGui::Button("Apply Motion and Layer"))
		{
			SetMotionType(motion);
			SetObjectLayer(layer);
		}
		
		ImGui::NewLine();

		// Display remove collider button
		if (ImGui::Button("Remove Collider Body"))
		{
			RemoveCollider();
		}
		
		return;
	}
	
	// ========== Collider type menu ==========
	const char* colliderTypes[] = { "Box", "Sphere", "Capsule", "Mesh", "Convex Hull" };
	static int selectedColliderIdx = 0; // Here we store our selection data as an index.

	const char* comboPreviewValue1 = colliderTypes[selectedColliderIdx];
	if (ImGui::BeginCombo("Collider Type", comboPreviewValue1, 0))
	{
		for (int n = 0; n < IM_COUNTOF(colliderTypes); n++)
		{
			const bool isSelected = (selectedColliderIdx == n);
			if (ImGui::Selectable(colliderTypes[n], isSelected))
			{
				selectedColliderIdx = n;
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	
	// ========== Collider settings ==========
	static float x = 0.0f;
	static float y = 0.0f;
	static float z = 0.0f;
	static float r = 0.0f;
	static float h = 0.0f;
	Droplet::GUID guid = 0;
	std::shared_ptr<Node> node = GetOwner();
	std::vector<std::shared_ptr<MeshComponent>> meshComponents = node->GetComponents<MeshComponent>();
	static std::size_t selectedMeshIdx = 0; // Here we store our selection data as an index.
	switch (selectedColliderIdx)
	{
	case 0: // Box collider
		{
			ImGui::Text("Input half extents:");
			ImGui::InputFloat("x", &x, 0.01f, 1.0f, "%.3f");
			ImGui::InputFloat("y", &y, 0.01f, 1.0f, "%.3f");
			ImGui::InputFloat("z", &z, 0.01f, 1.0f, "%.3f");
			break;
		}
	case 1: // Sphere collider
		{
			ImGui::Text("Input radius:");
			ImGui::InputFloat("Radius", &r, 0.01f, 1.0f, "%.3f");
			break;
		}
	case 2: // Capsule collider
		{
			ImGui::Text("Input capsule radius and cylinder height:");
			ImGui::InputFloat("Radius", &r, 0.01f, 1.0f, "%.3f");
			ImGui::InputFloat("Cylinder height", &h, 0.1f, 1.0f, "%.3f");
			break;
		}
	case 3: // Mesh collider
		{   
			if (meshComponents.empty())
			{
				ImGui::Text("There's no mesh component on this node!");
			}
			else
			{
				ImGui::Text("Choose mesh:");
				
				std::vector<std::string> meshGuids;
				meshGuids.reserve(meshComponents.size());
				for (const std::shared_ptr<MeshComponent> &m : meshComponents)
				{
					meshGuids.push_back(std::to_string(m->GetMeshGUID()));
				}

				// Pass in the preview value visible before opening the combo (it could technically be different contents or not pulled from items[])
				const char* comboPreviewValue2 = meshGuids[selectedMeshIdx].c_str();
				if (ImGui::BeginCombo("Mesh GUID", comboPreviewValue2, 0))
				{
					for (std::size_t n = 0; n < meshComponents.size(); n++)
					{
						const bool isSelected = (selectedMeshIdx == n);
						if (ImGui::Selectable(meshGuids[n].c_str(), isSelected))
						{
							selectedMeshIdx = n;
						}

						// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
			}
			
			break;
		}
	case 4:
		{
			// TODO: Implement
			break;
		}
	default:
		break;
	}
	
	static float offsetX = 0.0f;
	static float offsetY = 0.0f;
	static float offsetZ = 0.0f;
	ImGui::Text("Input offset:");
	ImGui::InputFloat("x##", &offsetX, 0.01f, 1.0f, "%.3f");
	ImGui::InputFloat("y##", &offsetY, 0.01f, 1.0f, "%.3f");
	ImGui::InputFloat("z##", &offsetZ, 0.01f, 1.0f, "%.3f");
	
	// ========== Collider apply button ==========
	static bool displayError = false;
	if (ImGui::Button("Apply collider"))
	{
		displayError = false;
		switch (selectedColliderIdx)
		{
		case 0: // Box collider
			if (x == 0.0f || y == 0.0f || z == 0.0f)
			{
				displayError = true;
				break;
			}
			displayError = false;
			SetBoxCollider(glm::vec3(x, y, z), JPH::Vec3(offsetX, offsetY, offsetZ));
			break;
		case 1: // Sphere collider
			if (r == 0.0f)
			{
				displayError = true;
				break;
			}
			displayError = false;
			SetSphereCollider(r, JPH::Vec3(offsetX, offsetY, offsetZ));
			break;
		case 2: // Capsule collider
			if (r == 0.0f || h == 0.0f)
			{
				displayError = true;
				break;
			}
			displayError = false;
			SetCapsuleCollider(r, h, JPH::Vec3(offsetX, offsetY, offsetZ));
			break;
		case 3: // Mesh collider
			SetMeshCollider(meshComponents[selectedMeshIdx]->GetMeshResource().Get(), JPH::Vec3(offsetX, offsetY, offsetZ));
			break;
		default:
			break;
		}
	}
	
	if (displayError)
	{
		ImGui::Text("Error: Invalid collider settings!");
	}
}

nlohmann::json PhysicsComponent::SerializeImpl()
{
	nlohmann::json json;
	
	// TODO: Serialize
	
	return json;
}

void PhysicsComponent::DeserializeImpl(nlohmann::json p_compJson)
{
	// TODO: Deserialize
}

void PhysicsComponent::CreateCollider(JPH::ShapeSettings *p_shapeSettings, const JPH::Vec3 &p_offset)
{
	if (m_physics == nullptr)
	{
		throw std::runtime_error("Physics system is null");
	}

	JPH::RotatedTranslatedShapeSettings offsetSettings(p_offset, JPH::Quat::sIdentity(), p_shapeSettings);
	
	JPH::ShapeSettings::ShapeResult result = offsetSettings.Create();

	if (result.HasError())
	{
		throw std::runtime_error(result.GetError().c_str());
	}

	m_shape = result.Get();
	JPH::BodyCreationSettings bodySettings(m_shape, GLMtoJPH(m_transform->GetPosition(Transform::Space::World)), 
		GLMtoJPH(m_transform->GetRotation(Transform::Space::World)), JPH::EMotionType::Static, Layers::NON_MOVING);
	bodySettings.mFriction = 0.2f;
	bodySettings.mAllowDynamicOrKinematic = true;
	bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
	bodySettings.mMassPropertiesOverride = JPH::MassProperties(1.0f);

	JPH::Body *body = m_physics->GetBodyInterface()->CreateBody(bodySettings);
	if (body == nullptr)
	{
		throw std::runtime_error("Failed to create Jolt body");
	}
	m_bodyID = body->GetID();

	m_physics->GetBodyInterface()->AddBody(m_bodyID, JPH::EActivation::Activate);
}

void PhysicsComponent::PushTransformToPhysics()
{
	if (m_shapeType != ShapeType::None)
	{
		return;
	}
	
	const JPH::Vec3 pos = GLMtoJPH(m_transform->GetPosition(Transform::Space::World));
	const JPH::Quat rot = GLMtoJPH(m_transform->GetRotation(Transform::Space::World));

	m_physics->GetBodyInterface()->SetPositionAndRotation(m_bodyID, pos, rot, JPH::EActivation::Activate);
}

void PhysicsComponent::PullTransformFromPhysics()
{
	if (m_shapeType != ShapeType::None)
	{
		return;
	}

	const JPH::RMat44 joltTransform = m_physics->GetBodyInterface()->GetWorldTransform(m_bodyID);

	const glm::vec3 pos = JPHtoGLM(joltTransform.GetTranslation());
	const glm::quat rot = JPHtoGLM(joltTransform.GetQuaternion());

	m_transform->SetPosition(pos);
	m_transform->SetRotation(rot);
}
