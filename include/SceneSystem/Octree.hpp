#pragma once

#include <glm\fwd.hpp>
#include <cstdint>
#include <memory>
#include <vector>
#include <array>
#include <string>
#include <math/bounds/AABB.hpp>
#include <math/bounds/Frustum.hpp>
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
		/// @param[out] p_outNodes The vector with Nodes to be rendered
		void GetNodesFromCulling(const Droplet::Math::Frustum &p_frustum, std::vector<std::shared_ptr<Node>> &p_outNodes);

		/// @brief Generates graphviz code for visualiziong the octree from top down (xz-projected)
		/// @note It is recommended to use nop/nop2 graphviz engine for this
		/// @return The string of graphviz code generated
		std::string ToGraphviz();

		/// @brief Generates graphviz code for visualiziong the octree as a tree
		/// @note It is recommended to use dot graphviz engine for this
		/// @return The string of graphviz code generated
		std::string ToGraphvizTree();

	private:
		struct TreeNode;

		/// @brief Helper function for updating Nodes in the octree by collecting dirty Nodes
		/// @param p_treeNode 
		/// @param[out] p_dirtyNodes A vector of all dirty Nodes
		void CollectDirtyNodes(const std::unique_ptr<TreeNode> &p_treeNode, std::vector<std::shared_ptr<Node>> &p_dirtyNodes);

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

		/// @brief Recursive helper function for generating graphviz code visualizing the octree from top down
		/// @param[in/out] p_data String of graphviz code
		/// @param p_treeNode The TreeNode to generate graphviz code for
		/// @param[in/out] p_nodeCounter A counter to name the graphviz elements
		void GenerateGraphvizLinks(std::string &p_data, std::unique_ptr<TreeNode> &p_treeNode, std::size_t &p_nodeCounter);

		/// @brief Recursive helper function for generating graphviz code visualizing the octree as a tree
		/// @param[in/out] p_data String of graphviz code
		/// @param p_treeNode The TreeNode to generate graphviz code for
		/// @param[in/out] p_nodeCounter A counter to name the graphviz elements
		/// @return The ID of the child node of the current p_treeNode
		std::size_t GenerateGraphvizLinksTree(std::string &p_data, std::unique_ptr<TreeNode> &p_treeNode, std::size_t &p_nodeCounter);

		/// @brief Helper function for generating graphvis code of an AABB
		/// @param p_aabb The AABB to turn into graphviz code
		/// @param p_nodeName The name of the graphviz element
		/// @param p_label The label printed in the visualization for the graphviz element
		/// @param p_color The color of the graphviz element
		/// @return A string of graphviz code representing the AABB
		std::string BoundingBoxToGraphviz(const Droplet::Math::AABB &p_aabb, const std::string &p_nodeName,
			const std::string &p_label, const std::string &p_color);

		static constexpr std::uint8_t C_MAX_CHILDREN = 8;
		static constexpr std::uint8_t C_MAX_DEPTH = 4;

		struct TreeNode
		{
			Droplet::Math::AABB octant;
			std::uint8_t level = 0;

			std::vector<std::shared_ptr<Node>> nodes;
			std::array<std::unique_ptr<TreeNode>, 8> children;
			bool isSubdivided = false;
		};

		bool m_isInitialized = false;

		std::unique_ptr<TreeNode> m_root;
	};
}