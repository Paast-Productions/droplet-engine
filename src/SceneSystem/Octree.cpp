#include "Octree.hpp"

#include <Node.hpp>
#include <glm\fwd.hpp>
#include <cstdint>
#include <memory>
#include <math/bounds/Intersection.hpp>

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
		m_root->volume = Droplet::Math::AABB(p_center, p_extents);
	}

	void Octree::Update()
	{
		if (!m_isInitialized)
		{
			return;
		}
	}

	void Octree::AddElement(const std::shared_ptr<Node> p_element)
	{
		if (!m_isInitialized)
		{
			return;
		}

		AddToTreeNode(p_element, m_root);
	}

	std::vector<const std::shared_ptr<Node>> Octree::GetNodesFromCulling(const Droplet::Math::Frustum &p_frustum)
	{
		std::vector<const std::shared_ptr<Node>> nodes;
		CheckIntersection(nodes, p_frustum, m_root.get());

		return nodes;
	}

	void Octree::AddToTreeNode(const std::shared_ptr<Node> p_element, std::unique_ptr<TreeNode> &p_node)
	{
		// Check if element bounding box intersects with TreeNode volume.
		if (Intersects(p_element->GetBoundingBox(), p_node->volume) == IntersectType::None)
		{
			return;
		}

		if (p_node->isLeafNode)
		{
			if (p_node->totalChildren < C_MAX_CHILDREN)
			{
				for (std::uint32_t i = 0; i < C_MAX_CHILDREN; i++)
				{
					if (p_node->children[i] == nullptr)
					{
						p_node->children[i] = std::make_unique<TreeNode>();
						p_node->children[i]->element = p_element;
						p_node->children[i]->volume = p_element->GetBoundingBox();
						p_node->totalChildren++;
						return;
					}
				}
			}
			else
			{
				AABB volumes[C_MAX_CHILDREN];
				SliceVolumeBoxes(p_node->volume, volumes);

				// Put all elements of the current tree node aside in a temporary container
				for (std::uint32_t i = 0; i < C_MAX_CHILDREN; i++)
				{
					std::shared_ptr<Node> node = p_node->children[i]->element;

					p_node->children[i] = std::make_unique<TreeNode>();
					p_node->children[i]->isLeafNode = false;
					p_node->children[i]->volume = volumes[i];
					p_node->children[i]->totalChildren = C_MAX_CHILDREN;
					p_node->children[i] = nullptr;

					AddToTreeNode(node, p_node->children[i]);
				}
			}
		}
	}

	void Octree::SliceVolumeBoxes(const Droplet::Math::AABB &p_parentVolume, Droplet::Math::AABB *p_childVolumes)
	{
		glm::vec3 c = p_parentVolume.center; // Center of the parent volume
		glm::vec3 h = p_parentVolume.extents.x / glm::vec3(2.0f); // Half of the parent volume extents

		p_childVolumes[0] = AABB(glm::vec3(c.x + h.x, c.y + h.y, c.z + h.z), h); // +x +y +z
		p_childVolumes[1] = AABB(glm::vec3(c.x - h.x, c.y + h.y, c.z + h.z), h); // -x +y +z
		p_childVolumes[2] = AABB(glm::vec3(c.x + h.x, c.y - h.y, c.z + h.z), h); // +x -y +z 
		p_childVolumes[3] = AABB(glm::vec3(c.x - h.x, c.y - h.y, c.z + h.z), h); // -x -y +z 
		p_childVolumes[4] = AABB(glm::vec3(c.x + h.x, c.y + h.y, c.z - h.z), h); // +x +y -z 
		p_childVolumes[5] = AABB(glm::vec3(c.x - h.x, c.y + h.y, c.z - h.z), h); // -x +y -z 
		p_childVolumes[6] = AABB(glm::vec3(c.x + h.x, c.y - h.y, c.z - h.z), h); // +x -y -z 
		p_childVolumes[7] = AABB(glm::vec3(c.x - h.x, c.y - h.y, c.z - h.z), h); // -x -y -z 
	}

	void Octree::CheckIntersection(std::vector<const std::shared_ptr<Node>> &p_nodes, const Droplet::Math::Frustum &p_frustum,
		const TreeNode *p_treeNode)
	{
		switch (Intersects(p_frustum, p_treeNode->volume))
		{
		case IntersectType::None:
			return;
		case IntersectType::Intersects:
			if (p_treeNode->isLeafNode) // TreeNode is a Node element
			{
				p_nodes.push_back(p_treeNode->element);
			}
			else // TreeNode is a volume box
			{
				for (std::uint32_t i = 0; i < C_MAX_CHILDREN; i++)
				{
					CheckIntersection(p_nodes, p_frustum, p_treeNode->children[i].get());
				}
			}
			break;
		case IntersectType::Contains:
			if (p_treeNode->isLeafNode) // TreeNode is a Node element
			{
				p_nodes.push_back(p_treeNode->element);
			}
			else // TreeNode is a volume box
			{
				AddAllNodeElements(p_nodes, p_treeNode);
			}
			break;
		}
	}

	void Octree::AddAllNodeElements(std::vector<const std::shared_ptr<Node>> &p_nodes, const TreeNode *p_treeNode)
	{
		if (p_treeNode->isLeafNode) // TreeNode is a Node element
		{
			p_nodes.push_back(p_treeNode->element);
		}
		else // TreeNode is a volume box
		{
			AddAllNodeElements(p_nodes, p_treeNode);
		}
	}
}