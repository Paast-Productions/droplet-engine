#include "Octree.hpp"

#include <glm\fwd.hpp>
#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <vector>
#include <math/bounds/Intersection.hpp>
#include <math/bounds/AABB.hpp>
#include <math/bounds/Frustum.hpp>
#include <SceneSystem/Node.hpp>
#include <SceneSystem/DefaultNodeBounds.hpp>

using namespace Droplet::Math;

namespace Droplet::Scene
{
	Octree::Octree(glm::vec3 p_center, glm::vec3 p_extents)
	{
		Initialize(p_center, p_extents);
	}

	void Octree::Initialize(glm::vec3 p_center, glm::vec3 p_extents)
	{
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
		if (!m_isInitialized)
		{
			return;
		}

		// Collect dirty nodes into a temporary list first to avoid mutating 
		// the octree during traversal (prevents iterator invalidation crashes)
		std::vector<std::shared_ptr<Node>> dirtyNodes;
		CollectDirtyNodes(m_root, dirtyNodes);

		for (const std::shared_ptr<Node> &dirty : dirtyNodes)
		{
			RemoveNode(dirty);
			AddNode(dirty);
		}
	}

	void Octree::AddNode(const std::shared_ptr<Node> p_node)
	{
		if (!m_isInitialized)
		{
			return;
		}

		AddToTreeNode(p_node, m_root);
	}

	void Octree::RemoveNode(const std::shared_ptr<Node> p_node)
	{
		if (!m_isInitialized)
		{
			return;
		}

		RemoveFromTreeNode(p_node, m_root);
	}

	std::vector<std::shared_ptr<Node>> Octree::GetNodesFromCulling(const Droplet::Math::Frustum &p_frustum)
	{
		std::vector<std::shared_ptr<Node>> nodes;
		CheckIntersection(nodes, p_frustum, m_root);

		return nodes;
	}

	void Octree::CollectDirtyNodes(const std::unique_ptr<TreeNode> &p_treeNode, std::vector<std::shared_ptr<Node>> &p_dirtyNodes)
	{
		if (p_treeNode == nullptr)
		{
			return;
		}

		// Check stored Nodes in the current TreeNode
		for (const std::shared_ptr<Node> &node : p_treeNode->nodes)
		{
			if (node != nullptr && node->GetTransform().IsDirty())
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

	void Octree::AddToTreeNode(const std::shared_ptr<Node> p_node, std::unique_ptr<TreeNode> &p_treeNode)
	{
		// Early outs
		if (p_node == nullptr || p_treeNode == nullptr || p_treeNode->level > C_MAX_DEPTH || 
			p_node->GetBounds()->Intersect(p_treeNode->octant) == IntersectType::None)
		{
			return;
		}

		// If already subdivided, pass directly to matching child octants
		if (p_treeNode->isSubdivided)
		{
			for (std::unique_ptr<TreeNode> &child : p_treeNode->children)
			{
				if (child != nullptr && p_node->GetBounds()->Intersect(child->octant) != IntersectType::None)
				{
					AddToTreeNode(p_node, child);
				}
			}
			return;
		}

		// Otherwise, store the node in this leaf
		p_treeNode->nodes.push_back(p_node);

		// Subdivide if capacity is exceeded and max depth is not reached
		if (p_treeNode->nodes.size() > C_MAX_CHILDREN && p_treeNode->level < C_MAX_DEPTH)
		{
			std::vector<AABB> octants;
			SubdivideOctant(p_treeNode->octant, octants);

			for (std::uint8_t i = 0; i < C_MAX_CHILDREN; i++)
			{
				p_treeNode->children[i] = std::make_unique<TreeNode>();
				p_treeNode->children[i]->octant = octants[i];
				p_treeNode->children[i]->level = p_treeNode->level + 1;
			}

			p_treeNode->isSubdivided = true;

			// Push existing Nodes down into matching child octants
			for (const std::shared_ptr<Node> &node : p_treeNode->nodes)
			{
				for (std::unique_ptr<TreeNode> &child : p_treeNode->children)
				{
					if (node->GetBounds()->Intersect(child->octant) != IntersectType::None)
					{
						AddToTreeNode(node, child);
					}
				}
			}

			p_treeNode->nodes.clear(); // Clear local storage after redistributing down
		}
	}

	void Octree::RemoveFromTreeNode(const std::shared_ptr<Node> p_node, std::unique_ptr<TreeNode> &p_treeNode)
	{
		if (p_node == nullptr || p_treeNode == nullptr)
		{
			return;
		}

		// Remove target Node from local storage if present
		std::vector<std::shared_ptr<Node>>::iterator it = std::remove(p_treeNode->nodes.begin(), p_treeNode->nodes.end(), p_node);
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

			// Bottom-Up Merging: Check if all child octants are empty
			bool canMerge = true;
			for (const std::unique_ptr<TreeNode> &child : p_treeNode->children)
			{
				if (child != nullptr)
				{
					if (child->isSubdivided || !child->nodes.empty())
					{
						canMerge = false;
						break;
					}
				}
			}

			// Reset children pointers and mark as un-subdivided
			if (canMerge)
			{
				for (std::unique_ptr<TreeNode> &child : p_treeNode->children)
				{
					child.reset();
				}
				p_treeNode->isSubdivided = false;
			}
		}
	}

	void Octree::SubdivideOctant(const AABB &p_parentOctant, std::vector<AABB> &p_childOctants)
	{
		glm::vec3 c = p_parentOctant.center;
		glm::vec3 h = p_parentOctant.extents / glm::vec3(2.0f);

		p_childOctants.emplace_back(AABB(glm::vec3(c.x + h.x, c.y + h.y, c.z + h.z), h)); // +x +y +z
		p_childOctants.emplace_back(AABB(glm::vec3(c.x - h.x, c.y + h.y, c.z + h.z), h)); // -x +y +z
		p_childOctants.emplace_back(AABB(glm::vec3(c.x + h.x, c.y - h.y, c.z + h.z), h)); // +x -y +z 
		p_childOctants.emplace_back(AABB(glm::vec3(c.x - h.x, c.y - h.y, c.z + h.z), h)); // -x -y +z 
		p_childOctants.emplace_back(AABB(glm::vec3(c.x + h.x, c.y + h.y, c.z - h.z), h)); // +x +y -z 
		p_childOctants.emplace_back(AABB(glm::vec3(c.x - h.x, c.y + h.y, c.z - h.z), h)); // -x +y -z 
		p_childOctants.emplace_back(AABB(glm::vec3(c.x + h.x, c.y - h.y, c.z - h.z), h)); // +x -y -z 
		p_childOctants.emplace_back(AABB(glm::vec3(c.x - h.x, c.y - h.y, c.z - h.z), h)); // -x -y -z 
	}

	void Octree::CheckIntersection(std::vector<std::shared_ptr<Node>> &p_nodes, const Frustum &p_frustum,
		const std::unique_ptr<TreeNode> &p_treeNode)
	{
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
			for (const std::shared_ptr<Node> &node : p_treeNode->nodes)
			{
				if (node != nullptr && node->GetBounds()->Intersect(p_frustum) != IntersectType::None)
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

	void Octree::AddAllNodeElements(std::vector<std::shared_ptr<Node>> &p_nodes, const std::unique_ptr<TreeNode> &p_treeNode)
	{
		if (p_treeNode == nullptr)
		{
			return;
		}

		for (const std::shared_ptr<Node> &node : p_treeNode->nodes)
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
}