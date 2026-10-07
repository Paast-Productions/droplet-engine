#pragma once

#include <glm\fwd.hpp>
#include <cstdint>
#include <memory>
#include <vector>
#include <math/bounds/AABB.hpp>
#include "SceneSystem/Node.hpp"

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

		/// @brief Adds a Node element to the octree
		/// @param p_node The Node element to be added to the octree
		void AddNode(const std::shared_ptr<Node> p_node);

		/// @brief Removes a Node element from the octree
		/// @param p_node The Node element to be removed
		void RemoveNode(const std::shared_ptr<Node> p_node);

		/// @brief Gets all Node instances intersecting with a frustum
		/// @param p_frustum The frustum to check for intersections
		std::vector<std::shared_ptr<Node>> GetNodesFromCulling(const Droplet::Math::Frustum &p_frustum);

	private:
		struct TreeNode;

		/// @brief Helper function for updating Nodes in the octree
		/// @param p_treeNode The TreeNode element to recurse, checking if needs to be updated
		void UpdateTreeNode(std::unique_ptr<TreeNode> &p_treeNode);

		/// @brief Helper function for adding elements to the octree
		/// @param p_node The Node to be added to the octree
		/// @param p_treeNode The TreeNode where the Node element should be set
		void AddToTreeNode(const std::shared_ptr<Node> p_node, std::unique_ptr<TreeNode> &p_treeNode);

		/// @brief Helper function for removing elements from the octree
		/// @param p_node The Node to be removed
		/// @param p_treeNode The TreeNode where to check for the Node element
		void RemoveFromTreeNode(const std::shared_ptr<Node> p_node, std::unique_ptr<TreeNode> &p_treeNode);

		/// @brief Subdivides a octant into eights
		/// @param p_parentOctant The octant to be subdivided
		/// @param[out] p_childOctants The subdivided octants
		void SubdivideOctant(const Droplet::Math::AABB &p_parentOctant, std::vector<Droplet::Math::AABB> &p_childOctants);

		/// @brief Helper function for getting Node instances intersecting with frustum
		/// @param[out] p_nodes The vector that contains all Node instances intersecting the frustum
		/// @param p_frustum The frustum to check for intersections
		/// @param p_treeNode The TreeNode instance to be checked for intersection
		void CheckIntersection(std::vector<std::shared_ptr<Node>> &p_nodes, const Droplet::Math::Frustum &p_frustum, 
			const std::unique_ptr<TreeNode> &p_treeNode);

		/// @brief Helper function for checking frustum culling
		/// @param[out] p_nodes The vector that contains all Node instances intersecting the frustum 
		/// @param p_treeNode The TreeNode instance to be checked for intersection
		void AddAllNodeElements(std::vector<std::shared_ptr<Node>> &p_nodes, const std::unique_ptr<TreeNode> &p_treeNode);

		static constexpr std::uint8_t C_MAX_CHILDREN = 8;
		static constexpr std::uint8_t C_MAX_DEPTH = 4;

		struct TreeNode
		{
			Droplet::Math::AABB octant;
			std::uint8_t level = 0;

			std::shared_ptr<Node> node;
			std::vector<std::unique_ptr<TreeNode>> children;
		};

		bool m_isInitialized = false;

		std::unique_ptr<TreeNode> m_root;
	};
}