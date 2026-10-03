#include "core/MeshBVH.hpp"
#include <vector>

#include <gtest/gtest.h>

using namespace Droplet;

namespace
{
	// Create an empty triangle mesh with no triangles
	const std::vector<glm::vec3> &EmptyTriangleMesh()
	{
		static std::vector<glm::vec3> mesh = {

		};

		return mesh;
	}

	// Create a simple triangle mesh with one triangle
	const std::vector<glm::vec3> &SimpleTriangleMesh()
	{
		static std::vector<glm::vec3> mesh = {
			{ 0.0f, 0.0f, 0.0f },	// Vertex 1
			{ 1.0f, 0.0f, 0.0f },	// Vertex 2
			{ 1.0f, 1.0f, 0.0f },	// Vertex 3
		};

		return mesh;
	}

	// Create a simple quad mesh with two triangles
	const std::vector<glm::vec3> &SimpleQuadMesh()
	{
		static std::vector<glm::vec3> mesh = {
			{ 0.0f, 0.0f, 0.0f },	// Vertex 1
			{ 1.0f, 0.0f, 0.0f },	// Vertex 2
			{ 1.0f, 1.0f, 0.0f },	// Vertex 3

			{ 0.0f, 0.0f, 0.0f },	// Vertex 1
			{ 1.0f, 1.0f, 0.0f },	// Vertex 3
			{ 0.0f, 1.0f, 0.0f },	// Vertex 4
		};

		return mesh;
	}

	// Create a simple cube mesh with 12 triangles
	const std::vector<glm::vec3> &SimpleCubeMesh()
	{
		static std::vector<glm::vec3> mesh = {
			// Front (z = 1)
			{0, 0, 1}, {1, 0, 1}, {1, 1, 1},
			{0, 0, 1}, {1, 1, 1}, {0, 1, 1},

			// Back (z = 0)
			{1, 0, 0}, {0, 0, 0}, {0, 1, 0},
			{1, 0, 0}, {0, 1, 0}, {1, 1, 0},

			// Left (x = 0)
			{0, 0, 0}, {0, 0, 1}, {0, 1, 1},
			{0, 0, 0}, {0, 1, 1}, {0, 1, 0},

			// Right (x = 1)
			{1, 0, 1}, {1, 0, 0}, {1, 1, 0},
			{1, 0, 1}, {1, 1, 0}, {1, 1, 1},

			// Bottom (y = 0)
			{0, 0, 0}, {1, 0, 0}, {1, 0, 1},
			{0, 0, 0}, {1, 0, 1}, {0, 0, 1},

			// Top (y = 1)
			{0, 1, 1}, {1, 1, 1}, {1, 1, 0},
			{0, 1, 1}, {1, 1, 0}, {0, 1, 0},
		};

		return mesh;
	}

	// Create a triangle mesh with a degenerate triangle
	const std::vector<glm::vec3> &InvalidTriangleMesh()
	{
		static std::vector<glm::vec3> mesh = {
			{ 0.0f, 0.0f, 0.0f },	// Vertex 1
			{ 0.0f, 0.0f, 0.0f },	// Vertex 2
			{ 1.0f, 1.0f, 0.0f },	// Vertex 3
		};

		return mesh;
	}
}

class MeshBVHTest : public ::testing::Test
{

};

class TestMeshBVH : public MeshBVH
{
public:
	std::shared_ptr<bvh::v2::Bvh<bvh::v2::Node<float, 3>>>	&GetBVH()				{ return m_bvh; }
	std::vector<bvh::v2::PrecomputedTri<float>>				&GetPrecomputedTris()	{ return m_precomputedTris; }
};

TEST(MeshBvhTest, ConstructSimpleBVH)
{
	TestMeshBVH bvh(SimpleTriangleMesh());
}
