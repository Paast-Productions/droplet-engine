#include <gtest/gtest.h>
#include <math/bounds/MeshBVH.hpp>
#include <vector>

using namespace Droplet;

class TestMeshBVH : public Math::MeshBVH
{
public:
	std::shared_ptr<bvh::v2::Bvh<bvh::v2::Node<float, 3>>>	&GetBVH()				{ return m_bvh; }
	std::vector<bvh::v2::PrecomputedTri<float>>				&GetPrecomputedTris()	{ return m_precomputedTris; }
};

class MeshBVHTest : public ::testing::Test
{
protected:

	void SetUp() override
	{

	}
};

namespace
{
	// Create an empty triangle mesh with no triangles
	const std::vector<glm::vec3> &EmptyTriangleMesh()
	{
		static std::vector<glm::vec3> mesh = {

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

	// Create a triangle mesh with one triangle
	const std::vector<glm::vec3> &TriangleMesh()
	{
		static std::vector<glm::vec3> mesh = {
			{ 0.0f, 0.0f, 0.0f },	// Vertex 1
			{ 1.0f, 0.0f, 0.0f },	// Vertex 2
			{ 1.0f, 1.0f, 0.0f },	// Vertex 3
		};

		return mesh;
	}

	// Create a quad mesh with two triangles
	const std::vector<glm::vec3> &QuadMesh()
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

	// Create a cube mesh with 12 triangles
	const std::vector<glm::vec3> &CubeMesh()
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
}


// Construction

TEST(MeshBvhTest, ConstructTriangleBVH)
{
	TestMeshBVH bvh(TriangleMesh());
}

// Raycasting

TEST(MeshBvhTest, RaycastTriangleHit)
{
	TestMeshBVH bvh(TriangleMesh());

	Math::Ray ray(glm::vec3(0.5f, 0.5f, -1.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	auto hit = bvh.Raycast(ray);

	EXPECT_TRUE(hit.didHit);
	EXPECT_NEAR(hit.distance, 1.0f, 1e-6f);
}

TEST(MeshBvhTest, RaycastTriangleMiss)
{
	TestMeshBVH bvh(TriangleMesh());

	Math::Ray ray(glm::vec3(1.5f, 1.5f, -1.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	auto hit = bvh.Raycast(ray);

	EXPECT_FALSE(hit.didHit);
}

TEST(MeshBvhTest, RaycastQuadHit)
{
	TestMeshBVH bvh(QuadMesh());
}
