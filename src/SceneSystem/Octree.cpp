#include "Octree.hpp"

#include <glm\fwd.hpp>
#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <unordered_set>
#include <math/bounds/Intersection.hpp>
#include <SceneSystem/Node.hpp>
#include <SceneSystem/bounds/NodeBounds.hpp>
#include <tracy/public/tracy/Tracy.hpp>
#include <Graphics/DebugDraw.hpp>
#include <GameInput.hpp>

using namespace Droplet::Math;

namespace Droplet::Scene
{
	Octree::Octree(glm::vec3 p_center, glm::vec3 p_extents)
	{
		Initialize(p_center, p_extents);
	}

	void Octree::Initialize(glm::vec3 p_center, glm::vec3 p_extents)
	{
		ZoneScoped;

		if (m_isInitialized)
		{
			return;
		}

		m_root = std::make_unique<TreeNode>();
		m_root->octant = AABB(p_center, p_extents);

		m_isInitialized = true;
	}

	void Octree::Update()
	{
		ZoneScoped;

		if (!m_isInitialized || m_root == nullptr)
		{
			return;
		}

		// Collect dirty nodes into a temporary list first to avoid mutating 
		// the octree during traversal (prevents iterator invalidation crashes)
		std::vector<std::shared_ptr<Node>> dirtyNodes; // TODO: Make weak pointer
		CollectDirtyNodes(m_root, dirtyNodes);
		
		// Removes duplicated nodes in dirtyNodes vector.
		std::ranges::sort(dirtyNodes.begin(), dirtyNodes.end());
		dirtyNodes.erase(std::ranges::unique(dirtyNodes).begin(), dirtyNodes.end());

		for (const std::shared_ptr<Node> &node : dirtyNodes)
		{
			if (node == nullptr || node->GetBounds() == nullptr)
			{
				continue;
			}
			
			const glm::mat4x4 matrix = node->GetTransform().GetMatrix(Transform::Space::World);
			const Math::AABB worldBounds = Math::AABB::Transform(node->GetBounds()->GetAABB(), matrix);
			
			// If the actual world bounds remain inside the previously inserted fat AABB,
			// the tree's placements are still conservative and no reinsertion is needed.
			if (m_placements.contains(node.get()))
			{
				continue;
			}
			
			if (node->GetBounds()->Intersect(worldBounds, matrix) == IntersectType::Contains)
			{
				continue;
			}
			
			RemoveFromTreeNode(node, m_root);
			const AABB fatBounds = AABB(worldBounds.center, worldBounds.extents + C_FAT_AABB_MARGIN);
			m_placements[node.get()] = fatBounds;
			AddToTreeNode(node, m_root, fatBounds);
		}
		
		if (GameInput::Get().KeyToggle(Key::KeyB)) // HACK: Should probably not be here
		{
			Render();
		}
	}

	void Octree::AddNode(const std::shared_ptr<Node> &p_node) // TODO: Make weak pointer
	{
		ZoneScoped;

		if (!m_isInitialized)
		{
			return;
		}
		
		const glm::mat4x4 matrix = p_node->GetTransform().GetMatrix(Transform::Space::World);
		const Math::AABB worldBounds = Math::AABB::Transform(p_node->GetBounds()->GetAABB(), matrix);

		const AABB fatBounds = AABB(worldBounds.center, worldBounds.extents + C_FAT_AABB_MARGIN);
		m_placements[p_node.get()] = fatBounds;
		AddToTreeNode(p_node, m_root, fatBounds);
	}

	void Octree::RemoveNode(const std::shared_ptr<Node> &p_node) // TODO: Make weak pointer
	{
		ZoneScoped;

		if (!m_isInitialized)
		{
			return;
		}

		RemoveFromTreeNode(p_node, m_root);
	}

	void Octree::GetNodesFromCulling(const Frustum &p_frustum, std::vector<std::shared_ptr<Node>> &p_outNodes) // TODO: Make weak pointer
	{
		ZoneScoped;

		CheckIntersection(p_outNodes, p_frustum, m_root);
	}

	void Octree::GetNodesFullyInFrustum(const Frustum &p_frustum, std::vector<std::shared_ptr<Node>> &p_outNodes) // TODO: Make weak pointer
	{
		ZoneScoped;

		std::vector<std::shared_ptr<Node>> found;
		CollectContained(found, p_frustum, m_root);

		// A node may be stored in several octants, so only keep unique ones
		std::ranges::sort(found.begin(), found.end());
		found.erase(std::ranges::unique(found).begin(), found.end());

		p_outNodes.insert(p_outNodes.end(), found.begin(), found.end());
	}

	void Octree::CollectContained(std::vector<std::shared_ptr<Node>> &p_nodes, const Frustum &p_frustum,
		const std::unique_ptr<TreeNode> &p_treeNode, bool p_octantContained) // TODO: Make weak pointer
	{
		ZoneScoped;

		if (p_treeNode == nullptr)
		{
			return;
		}

		// Child octants lie within their parent, so if the parent is fully inside the frustum, so are they and
		// the octant test can be skipped for the whole subtree.
		// Nodes stored in an octant may extend outside of it, so they are still tested individually, and
		// octants that are only touched must still be searched.
		bool octantContained = p_octantContained;

		if (!octantContained)
		{
			const IntersectType octantIntersection = Intersects(p_frustum, p_treeNode->octant);

			if (octantIntersection == IntersectType::None)
			{
				return;
			}

			octantContained = octantIntersection == IntersectType::Contains;
		}

		for (const std::shared_ptr<Node> &node : p_treeNode->nodes) // TODO: Make weak pointer
		{
			if (node == nullptr || node->GetBounds() == nullptr)
			{
				continue;
			}

			if (node->GetBounds()->Intersect(p_frustum, node->GetTransform().GetMatrix(Transform::Space::World)) == IntersectType::Contains)
			{
				p_nodes.push_back(node);
			}
		}

		if (p_treeNode->isSubdivided)
		{
			for (const std::unique_ptr<TreeNode> &child : p_treeNode->children)
			{
				CollectContained(p_nodes, p_frustum, child, octantContained);
			}
		}
	}

	std::weak_ptr<Node> Octree::GetNodeFromRaycast(const Ray &p_ray, RayHit *p_hit)
	{
		RayHit hit{};
		std::weak_ptr<Node> node{};

		RaycastNode(node, hit, p_ray, m_root);

		if (p_hit != nullptr)
		{
			*p_hit = hit;
		}

		return node;
	}

	std::vector<std::weak_ptr<Node>> Octree::GetNodesFromRaycast(const Ray &p_ray)
	{
		std::vector<std::weak_ptr<Node>> nodes{};
		RaycastNodes(nodes, p_ray, m_root);
		return nodes;
	}

	std::string Octree::ToGraphviz()
	{
		ZoneScoped;

		std::string toReturn = "digraph D{\n";

		size_t counter = 0;
		GenerateGraphvizLinks(toReturn, m_root, counter);

		return toReturn + "}\n";
	}

	std::string Octree::ToGraphvizTree()
	{
		ZoneScoped;

		std::string toReturn = "digraph D{\n";

		size_t counter = 0;
		toReturn += std::to_string(counter) + "[label = \"Root\"]\n";
		GenerateGraphvizLinksTree(toReturn, m_root, counter);

		return toReturn + "}\n";
	}
	
	void Octree::Render()
	{
		RenderOctant(m_root);
	}

	void Octree::CollectDirtyNodes(const std::unique_ptr<TreeNode> &p_treeNode, std::vector<std::shared_ptr<Node>> &p_dirtyNodes) // TODO: Make weak pointer
	{
		ZoneScoped;

		if (p_treeNode == nullptr)
		{
			return;
		}

		// Check stored Nodes in the current TreeNode
		for (const std::shared_ptr<Node> &node : p_treeNode->nodes) // TODO: Make weak pointer
		{
			if (node != nullptr && node->GetTransform().HasBeenChanged())
			{
				p_dirtyNodes.push_back(node);
			}
		}

		// Recurse into children if subdivided
		if (p_treeNode->isSubdivided)
		{
			for (const std::unique_ptr<TreeNode> &child : p_treeNode->children)
			{
				if (child != nullptr)
				{
					CollectDirtyNodes(child, p_dirtyNodes);
				}
			}
		}
	}

	void Octree::AddToTreeNode(const std::shared_ptr<Node> &p_node, const std::unique_ptr<TreeNode> &p_treeNode, const AABB &p_placementBounds) // TODO: Make weak pointer
	{
		ZoneScoped;

		// Early outs
		if (p_node == nullptr)
		{
			return;
		}

		if (p_treeNode == nullptr)
		{
			return;
		}

		if (p_treeNode->level > C_MAX_DEPTH)
		{
			return;
		}

		const IntersectType intersection = p_node->GetBounds()->Intersect(
			p_treeNode->octant, 
			p_node->GetTransform().GetMatrix(Transform::Space::World)
		);
		if (intersection == IntersectType::None)
		{
			return;
		}

		// If already subdivided, pass directly to matching child octants
		if (p_treeNode->isSubdivided)
		{
			for (std::unique_ptr<TreeNode> &child : p_treeNode->children)
			{
				if (child == nullptr)
				{
					continue;
				}
				
				if (Intersects(p_placementBounds, child->octant) != IntersectType::None)
				{
					AddToTreeNode(p_node, child, p_placementBounds);
				}
			}
			return;
		}

		// Otherwise, store the node in this leaf
		p_treeNode->nodes.push_back(p_node);

		// Subdivide if capacity is exceeded and max depth is not reached
		if (p_treeNode->nodes.size() > C_MAX_NODES && p_treeNode->level < C_MAX_DEPTH)
		{
			std::array<AABB, 8> octants;
			SubdivideOctant(p_treeNode->octant, octants);

			for (std::uint8_t i = 0; i < 8; i++)
			{
				p_treeNode->children[i] = std::make_unique<TreeNode>();
				p_treeNode->children[i]->octant = octants[i];
				p_treeNode->children[i]->level = p_treeNode->level + 1;
			}

			p_treeNode->isSubdivided = true;

			// Re-distribute every stored node using its own cached placement bounds
			const std::vector<std::shared_ptr<Node>> existingNodes = p_treeNode->nodes;
			p_treeNode->nodes.clear();
			
			for (const std::shared_ptr<Node> &node : existingNodes)
			{
				if (node == nullptr)
				{
					continue;
				}
				
				auto placement = m_placements.find(node.get());
				const AABB nodePlacement = placement != m_placements.end() ? placement->second : 
					AABB::Transform(node->GetBounds()->GetAABB(), node->GetTransform().GetMatrix(Transform::Space::World));
				
				for (std::unique_ptr<TreeNode> &child : p_treeNode->children)
				{
					if (child == nullptr)
					{
						continue;
					}
					
					if (Intersects(nodePlacement, child->octant) != IntersectType::None)
					{
						AddToTreeNode(node, child, nodePlacement);
					}
				}
			}
		}
	}

	void Octree::RemoveFromTreeNode(const std::shared_ptr<Node> &p_node, const std::unique_ptr<TreeNode> &p_treeNode) // TODO: Make weak pointer
	{
		ZoneScoped;

		if (p_node == nullptr || p_treeNode == nullptr)
		{
			return;
		}

		// Remove target Node from local storage if present
		auto it = std::ranges::remove(p_treeNode->nodes, p_node).begin();
		if (it != p_treeNode->nodes.end())
		{
			p_treeNode->nodes.erase(it, p_treeNode->nodes.end());
		}

		// Recurse down if subdivided
		if (p_treeNode->isSubdivided)
		{
			for (std::unique_ptr<TreeNode> &child : p_treeNode->children)
			{
				if (child != nullptr)
				{
					RemoveFromTreeNode(p_node, child);
				}
			}

			TryMerge(p_treeNode);
		}
	}

	void Octree::SubdivideOctant(const AABB &p_parentOctant, std::array<AABB, 8> &p_childOctants)
	{
		ZoneScoped;

		glm::vec3 c = p_parentOctant.center;
		glm::vec3 h = p_parentOctant.extents / glm::vec3(2.0f);

		p_childOctants[0] = AABB(glm::vec3(c.x + h.x, c.y + h.y, c.z + h.z), h); // +x +y +z
		p_childOctants[1] = AABB(glm::vec3(c.x - h.x, c.y + h.y, c.z + h.z), h); // -x +y +z
		p_childOctants[2] = AABB(glm::vec3(c.x + h.x, c.y - h.y, c.z + h.z), h); // +x -y +z 
		p_childOctants[3] = AABB(glm::vec3(c.x - h.x, c.y - h.y, c.z + h.z), h); // -x -y +z 
		p_childOctants[4] = AABB(glm::vec3(c.x + h.x, c.y + h.y, c.z - h.z), h); // +x +y -z 
		p_childOctants[5] = AABB(glm::vec3(c.x - h.x, c.y + h.y, c.z - h.z), h); // -x +y -z 
		p_childOctants[6] = AABB(glm::vec3(c.x + h.x, c.y - h.y, c.z - h.z), h); // +x -y -z 
		p_childOctants[7] = AABB(glm::vec3(c.x - h.x, c.y - h.y, c.z - h.z), h); // -x -y -z 
	}

	void Octree::CheckIntersection(std::vector<std::shared_ptr<Node>> &p_nodes, const Frustum &p_frustum,
		const std::unique_ptr<TreeNode> &p_treeNode) // TODO: Make weak pointer
	{
		ZoneScoped;

		if (p_treeNode == nullptr)
		{
			return;
		}

		switch (Intersects(p_frustum, p_treeNode->octant))
		{
		case IntersectType::None:
			return;
		case IntersectType::Intersects:
			// Check individual nodes stored in this leaf
			for (const std::shared_ptr<Node> &node : p_treeNode->nodes) // TODO: Make weak pointer
			{
				if (node == nullptr)
				{
					continue;
				}

				if (node->GetBounds()->Intersect(p_frustum, node->GetTransform().GetMatrix(Transform::Space::World)) != IntersectType::None)
				{
					p_nodes.push_back(node);
				}
			}

			// Recurse into child octants
			if (p_treeNode->isSubdivided)
			{
				for (const std::unique_ptr<TreeNode> &child : p_treeNode->children)
				{
					if (child != nullptr)
					{
						CheckIntersection(p_nodes, p_frustum, child);
					}
				}
			}
			break;
		case IntersectType::Contains:
			AddAllNodeElements(p_nodes, p_treeNode);
			break;
		}
	}

	void Octree::RaycastNode(std::weak_ptr<Node> &p_node, Math::RayHit &p_hit, const Math::Ray &p_ray, const std::unique_ptr<TreeNode> &p_treeNode)
	{
		if (p_treeNode == nullptr)
		{
			return;
		}

		RayHit octantHit = Raycast(p_ray, p_treeNode->octant);
		if (!octantHit.didHit)
		{
			return;
		}

		if (p_treeNode->isSubdivided)
		{
			// Recurse into child octants

			for (const std::unique_ptr<TreeNode> &child : p_treeNode->children)
			{
				if (child == nullptr)
				{
					continue;
				}

				RaycastNode(p_node, p_hit, p_ray, child);
			}

			return;
		}
		
		p_hit.distance = std::numeric_limits<float>::max();

		// Check individual nodes stored in this leaf
		for (const std::shared_ptr<Node> &node : p_treeNode->nodes) // TODO: Make weak pointer
		{
			if (node == nullptr)
			{
				continue;
			}

			RayHit nodeHit = node->GetBounds()->Raycast(p_ray, node->GetTransform().GetMatrix(Transform::Space::World));

			if (!nodeHit.didHit)
			{
				continue;
			}

			if (nodeHit.distance < p_hit.distance)
			{
				p_hit = nodeHit;
				p_node = node;
			}
		}
	}

	void Octree::RaycastNodes(std::vector<std::weak_ptr<Node>> &p_nodes, const Math::Ray &p_ray, const std::unique_ptr<TreeNode> &p_treeNode)
	{
		if (p_treeNode == nullptr)
		{
			return;
		}

		RayHit octantHit = Raycast(p_ray, p_treeNode->octant);
		if (!octantHit.didHit)
		{
			return;
		}

		if (p_treeNode->isSubdivided)
		{
			// Recurse into child octants

			for (const std::unique_ptr<TreeNode> &child : p_treeNode->children)
			{
				if (child == nullptr)
				{
					continue;
				}

				RaycastNodes(p_nodes, p_ray, child);
			}

			return;
		}

		// Check individual nodes stored in this leaf
		for (const std::shared_ptr<Node> &node : p_treeNode->nodes) // TODO: Make weak pointer
		{
			if (node == nullptr)
			{
				continue;
			}

			RayHit nodeHit = node->GetBounds()->Raycast(p_ray, node->GetTransform().GetMatrix(Transform::Space::World));

			if (!nodeHit.didHit)
			{
				continue;
			}

			p_nodes.push_back(node);
		}
	}

	void Octree::AddAllNodeElements(std::vector<std::shared_ptr<Node>> &p_nodes, const std::unique_ptr<TreeNode> &p_treeNode) // TODO: Make weak pointer
	{
		ZoneScoped;

		if (p_treeNode == nullptr)
		{
			return;
		}

		for (const std::shared_ptr<Node> &node : p_treeNode->nodes) // TODO: Make weak pointer
		{
			p_nodes.push_back(node);
		}

		if (p_treeNode->isSubdivided)
		{
			for (const std::unique_ptr<TreeNode> &child : p_treeNode->children)
			{
				if (child != nullptr)
				{
					AddAllNodeElements(p_nodes, child);
				}
			}
		}
	}

	void Octree::TryMerge(const std::unique_ptr<TreeNode> &p_treeNode)
	{
		ZoneScoped;
		
		if (!p_treeNode->isSubdivided)
		{
			return;
		}
		
		std::vector<std::shared_ptr<Node>> uniqueNodes;
		std::unordered_set<Node *> seenNodes;
		
		// We only need to know whether the count exceeds the merge threshold
		bool exceedsThreshold = false;
		
		const auto collectNodes = [&](const auto &self, const std::unique_ptr<TreeNode> &treeNode) -> void
		{
			if (exceedsThreshold)
			{
				return;
			}

			for (const std::shared_ptr<Node> &node : treeNode->nodes)
			{
				if (node == nullptr)
				{
					continue;
				}

				if (seenNodes.insert(node.get()).second)
				{
					uniqueNodes.push_back(node);

					if (uniqueNodes.size() > C_MERGE_THRESHOLD)
					{
						exceedsThreshold = true;
						return;
					}
				}
			}

			if (treeNode->isSubdivided)
			{
				for (const std::unique_ptr<TreeNode> &child : treeNode->children)
				{
					if (child != nullptr)
					{
						self(self, child);

						if (exceedsThreshold)
						{
							return;
						}
					}
				}
			}
		};

		for (const std::unique_ptr<TreeNode> &child : p_treeNode->children)
		{
			if (child != nullptr)
			{
				collectNodes(collectNodes, child);

				if (exceedsThreshold)
				{
					return;
				}
			}
		}
		
		// Too many unique nodes remain, keep the subdivision
		if (uniqueNodes.size() > C_MERGE_THRESHOLD)
		{
			return;
		}

		// Collapse all children into this parent
		p_treeNode->nodes = std::move(uniqueNodes);

		for (std::unique_ptr<TreeNode> &child : p_treeNode->children)
		{
			child.reset();
		}

		p_treeNode->isSubdivided = false;
	}

	void Octree::GenerateGraphvizLinks(std::string &p_data, std::unique_ptr<TreeNode> &p_treeNode, std::size_t &p_nodeCounter)
	{
		if (p_treeNode == nullptr)
		{
			return;
		}

		std::string octantName = "Octant" + std::to_string(p_nodeCounter++);
		p_data += BoundingBoxToGraphviz(p_treeNode->octant, octantName, "", "black");

		if (!p_treeNode->nodes.empty())
		{
			for (std::shared_ptr<Node> &node : p_treeNode->nodes) // TODO: Make weak pointer
			{
				std::string nodeName = "Node" + std::to_string(p_nodeCounter++);

				p_data += BoundingBoxToGraphviz(
					AABB::Transform(
						node->GetBounds()->GetAABB(), 
						node->GetTransform().GetMatrix(Transform::Space::World)
					),
					nodeName, 
					node->GetName(), 
					"blue"
				);
			}
		}

		if (p_treeNode->children[0] != nullptr || p_treeNode->nodes.empty())
		{
			for (std::unique_ptr<TreeNode> &child : p_treeNode->children)
			{
				GenerateGraphvizLinks(p_data, child, p_nodeCounter);
			}
		}
	}

	std::size_t Octree::GenerateGraphvizLinksTree(std::string &p_data, const std::unique_ptr<TreeNode> &p_treeNode, std::size_t &p_nodeCounter)
	{
		std::size_t myID = p_nodeCounter + 1;

		if (p_treeNode == nullptr)
		{
			return myID;
		}

		for (std::shared_ptr<Node> &node : p_treeNode->nodes) // TODO: Make weak pointer
		{
			p_data += std::to_string(myID) + "[label = \"" + node->GetName() + "\"]\n";
			p_data += std::to_string(p_nodeCounter) + " -> " + std::to_string(myID) + "\n";
			myID++;
		}
		p_nodeCounter += p_treeNode->nodes.size();

		if (p_treeNode->children[0] != nullptr)
		{
			for (std::unique_ptr<TreeNode> &child : p_treeNode->children)
			{
				std::string childID = std::to_string(GenerateGraphvizLinksTree(p_data, child, ++p_nodeCounter));
				p_data += std::to_string(myID) + " -> " + childID + '\n';
			}
		}

		return myID;
	}

	std::string Octree::BoundingBoxToGraphviz(const Droplet::Math::AABB &p_aabb, const std::string &p_nodeName, 
		const std::string &p_label, const std::string &p_color)
	{
		float scale = 1.0f / 10.0f;
		float width = p_aabb.extents.x * 2 * scale;
		float height = p_aabb.extents.z * 2 * scale;
		float centerX = p_aabb.center.x * scale;
		float centerY = p_aabb.center.z * scale;

		// Points per inch. Graphviz uses inches for node width and height attributes, 
		// but points (72 points per inch) for layout pos coordinates.
		const std::uint32_t ppi = 72; 
		std::string toReturn = p_nodeName + " [label=\"" + p_label + "\", color=\"" + p_color + "\", fixedsize=true, shape=box, width="
			+ std::to_string(width) + ", height=" + std::to_string(height) +
			", pos=\"" + std::to_string(centerX * ppi) + ',' + std::to_string(centerY * ppi) + "!\"]\n";

		return toReturn;
	}
	
	void Octree::RenderOctant(const std::unique_ptr<TreeNode>& p_treeNode)
	{
		if (p_treeNode == nullptr)
		{
			return;
		}
		
		Graphics::Debug::DebugDraw::DrawBox(p_treeNode->octant, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), true, true);
		
		for (const std::shared_ptr<Node> &node : p_treeNode->nodes)
		{
			if (node != nullptr)
			{
				OBB obb = node->GetBounds()->GetTransformedOBB(node->GetTransform().GetMatrix(Transform::Space::World));
				Graphics::Debug::DebugDraw::DrawBox(obb, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), true, true);
			}
		}
		
		for (const std::unique_ptr<TreeNode> &treeNode : p_treeNode->children)
		{
			RenderOctant(treeNode);
		}
	}
}
