#pragma once

#include <glm\fwd.hpp>
#include <cstdint>
#include <memory>
#include <math/bounds/AABB.hpp>

#include "Node.hpp"

namespace Droplet::Scene
{
	class Octree
	{
	public:
		Octree() = default;
		/// @brief Constructs the octree using Initialize()
		/// @param p_center The center position of the octree root AABB
		/// @param p_extents The cubic radius of the octree root AABB
		Octree(glm::vec3 p_center, glm::vec3 p_extents);
		~Octree() = default;

		/// @brief Initializes the octree
		/// @param p_center The center position of the octree root AABB
		/// @param p_extents The cubic radius of the octree root AABB
		void Initialize(glm::vec3 p_center, glm::vec3 p_extents);

		/// @brief Updates the octree
		void Update();

		/// @brief Adds a node element to the octree
		/// @param p_element The element to be added to the octree
		void AddElement(const std::shared_ptr<Node> p_element);

		//void RemoveElement()

		// TODO: Implement getting list of nodes from frustum culling
		// std::vector<const std::shared_ptr<Node>> GetNodesFromCulling(p_frustum);

	private:
		struct TreeNode;

		/// @brief Helper function for adding elements to the octree
		/// @param p_element The element to be added to the octree
		/// @param p_node The node where the element should be set
		void AddToTreeNode(const std::shared_ptr<Node> p_element, std::unique_ptr<TreeNode> &p_node);

		/// @brief Slices a bounding volume into eights
		/// @param p_boundingBox The box to be sliced
		void SliceVolumeBoxes(const Droplet::Math::AABB &p_parentVolume, Droplet::Math::AABB *p_childVolumes);

		static constexpr std::uint32_t C_MAX_CHILDREN = 8;
		static constexpr std::uint32_t C_MAX_DEPTH = 4;

		struct TreeNode
		{
			bool isLeafNode = true;
			Droplet::Math::AABB volume;

			std::shared_ptr<Node> element;
			std::uint32_t totalChildren = 0;
			std::unique_ptr<TreeNode> children[C_MAX_CHILDREN] = { nullptr };
		};

		bool m_isInitialized = false;

		std::unique_ptr<TreeNode> m_root;
	};
}