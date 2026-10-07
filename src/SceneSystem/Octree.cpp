#include "Octree.hpp"

#include <glm\fwd.hpp>
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

	void Octree::CollectDirtyNodes(std::unique_ptr<TreeNode> &p_treeNode, std::vector<std::shared_ptr<Node>> &p_dirtyNodes)
	{
		if (p_treeNode == nullptr)
		{
			return;
		}

		for (std::unique_ptr<TreeNode> &child : p_treeNode->children)
		{
			CollectDirtyNodes(child, p_dirtyNodes);

			if (child->node->GetTransform().IsDirty())
			{
				p_dirtyNodes.push_back(child->node);
			}
		}
	}

	void Octree::AddToTreeNode(const std::shared_ptr<Node> p_node, std::unique_ptr<TreeNode> &p_treeNode)
	{
		if (p_treeNode->level > C_MAX_DEPTH)
		{
			// Log info/warning: Node could not be added to octree due to max depth has been reached.
			return;
		}

		// Check if element bounding box intersects with TreeNode volume.
		if (p_node->GetBounds()->Intersect(p_treeNode->octant) == IntersectType::None)
		{
			// Log info/warning: Node could not be added to octree due to Node being outside the octree.
			return;
		}

		if (p_treeNode->node == nullptr) // Adds the node to the tree
		{
			if (p_treeNode->children.size() < C_MAX_CHILDREN)
			{
				p_treeNode->children.emplace_back(std::make_unique<TreeNode>());
				p_treeNode->children.back()->level = p_treeNode->level + 1;
				p_treeNode->children.back()->node = p_node;
			}
			else // Subdivides the octant
			{
				std::vector<AABB> octants;
				SubdivideOctant(p_treeNode->octant, octants);

				for (std::uint8_t i = 0; i < C_MAX_CHILDREN; i++)
				{
					std::shared_ptr<Node> node = p_treeNode->children[i]->node;

					p_treeNode->children[i] = std::make_unique<TreeNode>();
					p_treeNode->children[i]->octant = octants[i];
					p_treeNode->children[i]->node = nullptr;

					AddToTreeNode(node, p_treeNode->children[i]);
				}
			}
		}

		// Current TreeNode is a parent octant
		// Add the Node to child octants of the current TreeNode
		for (std::unique_ptr<TreeNode> &c : p_treeNode->children)
		{
			AddToTreeNode(p_node, c);
		}
	}

	void Octree::RemoveFromTreeNode(const std::shared_ptr<Node> p_node, std::unique_ptr<TreeNode> &p_treeNode)
	{
		if (p_node == nullptr || p_treeNode == nullptr)
		{
			return;
		}

		if (p_treeNode->node == p_node) // The TreeNode contains the target Node
		{
			p_treeNode->node = nullptr;
		}
		else if (!p_treeNode->children.empty()) // The TreeNode is an octant
		{
			for (std::unique_ptr<TreeNode> &child : p_treeNode->children)
			{
				RemoveFromTreeNode(p_node, child);
			}

			// Check if all children are empty for merging child octants
			bool canMerge = true;
			for (const std::unique_ptr<TreeNode> &child : p_treeNode->children)
			{
				// A child is not empty, don't merge child octants
				if (child->node != nullptr || !child->children.empty())
				{
					canMerge = false;
					break;
				}
			}

			// Merge child octants by destroying the children
			if (canMerge)
			{
				p_treeNode->children.clear();
			}
		}
	}

	void Octree::SubdivideOctant(const AABB &p_parentOctant, std::vector<AABB> &p_childOctants)
	{
		glm::vec3 c = p_parentOctant.center; // Center of the parent volume
		glm::vec3 h = p_parentOctant.extents / glm::vec3(2.0f); // Half of the parent volume extents

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
		switch (Intersects(p_frustum, p_treeNode->octant))
		{
		case IntersectType::None:
			return;
		case IntersectType::Intersects:
			if (p_treeNode->node != nullptr) // TreeNode is a Node element
			{
				p_nodes.push_back(p_treeNode->node);
			}
			else // TreeNode is a volume box
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
			if (p_treeNode->node != nullptr) // TreeNode is a Node element
			{
				p_nodes.push_back(p_treeNode->node);
			}
			else // TreeNode is a volume box
			{
				AddAllNodeElements(p_nodes, p_treeNode);
			}
			break;
		}
	}

	void Octree::AddAllNodeElements(std::vector<std::shared_ptr<Node>> &p_nodes, const std::unique_ptr<TreeNode> &p_treeNode)
	{
		if (p_treeNode->node != nullptr) // TreeNode is a Node element
		{
			p_nodes.push_back(p_treeNode->node);
		}
		else // TreeNode is a volume box
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