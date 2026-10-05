#include "Octree.hpp"

#include <Node.hpp>
#include <glm\fwd.hpp>
#include <cstdint>
#include <memory>

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
		// Check if element bounding box intersects with TreeNode.
		// If not, return.
		
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
}