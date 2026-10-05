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
		
		// Check if current tree node is leaf node.
			// Check if current tree node is not full.
			// If not full:
				// Find an empty child node of current node and set p_element as child.
			// If full:
				// Save the children (elements) of current node in a temporary container.
				// Slice the current volume box into 8.
				// Replace the current children with the new smaller volume boxes.
				// For each of the new child nodes of the current node, AddToTreeNode using
				// every element and TreeNode.
				// Set the child node to nullptr.
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
}