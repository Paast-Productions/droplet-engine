#include <gtest/gtest.h>
#include <math/bounds/Intersection.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace Droplet::Math;

namespace
{
	constexpr float EPSILON = 1e-6f;

	Ray MakeBaseRay()
	{
		return Ray(
			glm::vec3(0.0f, 0.0f, 0.0f), 
			glm::vec3(0.0f, 0.0f, 1.0f)
		);
	}

	Plane MakeBasePlane()
	{
		return Plane(
			glm::vec3(0.0f, 1.0f, 0.0f),
			0.0f
		);
	}

	AABB MakeBaseAABB()
	{
		return AABB(
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(1.0f, 1.0f, 1.0f)
		);
	}

	OBB MakeBaseOBB()
	{
		return OBB(
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::quat(1.0f, 0.0f, 0.0f, 0.0f)
		);
	}

	Sphere MakeBaseSphere()
	{
		return Sphere(
			glm::vec3(0.0f, 0.0f, 0.0f),
			1.0f
		);
	}

	Frustum MakeBaseFrustum()
	{
		return Frustum(
			Plane(glm::vec3(1.0f, 0.0f, 0.0f), -1.0f),
			Plane(glm::vec3(-1.0f, 0.0f, 0.0f), -1.0f),
			Plane(glm::vec3(0.0f, -1.0f, 0.0f), -1.0f),
			Plane(glm::vec3(0.0f, 1.0f, 0.0f), -1.0f),
			Plane(glm::vec3(0.0f, 0.0f, 1.0f), -1.0f),
			Plane(glm::vec3(0.0f, 0.0f, -1.0f), -1.0f)
		);
	}

	Frustum MakeCameraFrustum(float p_fov, float p_aspect, float p_near, float p_far)
	{
		return Frustum(
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			p_fov, p_aspect, p_near, p_far
		);
	}
}


#pragma region Raycasting

TEST(IntersectionTest, RaycastPlaneHit)
{
	Plane plane = MakeBasePlane();

	Ray ray = Ray(
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::normalize(glm::vec3(0.0f, -1.0f, 1.0f))
	);

	RayHit hit = Raycast(ray, plane);

	EXPECT_TRUE(hit.didHit);
	EXPECT_NEAR(hit.distance, sqrtf(2.0f), EPSILON);

	EXPECT_NEAR(hit.point.x, 0.0f, EPSILON);
	EXPECT_NEAR(hit.point.y, 0.0f, EPSILON);
	EXPECT_NEAR(hit.point.z, 1.0f, EPSILON);

	EXPECT_NEAR(hit.normal.x, 0.0f, EPSILON);
	EXPECT_NEAR(hit.normal.y, 1.0f, EPSILON);
	EXPECT_NEAR(hit.normal.z, 0.0f, EPSILON);
}

TEST(IntersectionTest, RaycastPlaneMiss)
{
	Plane plane = MakeBasePlane();

	Ray rayAway = Ray(
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	RayHit hitAway = Raycast(rayAway, plane);

	EXPECT_FALSE(hitAway.didHit);

	Ray rayParallel = Ray(
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);

	RayHit hitParallel = Raycast(rayParallel, plane);

	EXPECT_FALSE(hitParallel.didHit);
}

TEST(IntersectionTest, RaycastAABBHit)
{
	AABB aabb = MakeBaseAABB();

	Ray ray = Ray(
		glm::vec3(-3.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f)
	);

	RayHit hit = Raycast(ray, aabb);

	EXPECT_TRUE(hit.didHit);
	EXPECT_NEAR(hit.distance, 2.0f, EPSILON);

	EXPECT_NEAR(hit.point.x, -1.0f, EPSILON);
	EXPECT_NEAR(hit.point.y, 0.0f, EPSILON);
	EXPECT_NEAR(hit.point.z, 0.0f, EPSILON);

	EXPECT_NEAR(hit.normal.x, -1.0f, EPSILON);
	EXPECT_NEAR(hit.normal.y, 0.0f, EPSILON);
	EXPECT_NEAR(hit.normal.z, 0.0f, EPSILON);
}

TEST(IntersectionTest, RaycastAABBMiss)
{
	AABB aabb = MakeBaseAABB();

	Ray ray = Ray(
		glm::vec3(-3.0f, 0.0f, 0.0f),
		glm::normalize(glm::vec3(1.0f, -1.0f, 0.0f))
	);

	RayHit hit = Raycast(ray, aabb);

	EXPECT_FALSE(hit.didHit);
}

TEST(IntersectionTest, RaycastOBBHit)
{
	// Base OBB
	OBB obb = MakeBaseOBB();

	Ray ray = Ray(
		glm::vec3(-3.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f)
	);

	RayHit hit = Raycast(ray, obb);

	EXPECT_TRUE(hit.didHit);
	EXPECT_NEAR(hit.distance, 2.0f, EPSILON);

	EXPECT_NEAR(hit.point.x, -1.0f, EPSILON);
	EXPECT_NEAR(hit.point.y, 0.0f, EPSILON);
	EXPECT_NEAR(hit.point.z, 0.0f, EPSILON);

	EXPECT_NEAR(hit.normal.x, -1.0f, EPSILON);
	EXPECT_NEAR(hit.normal.y, 0.0f, EPSILON);
	EXPECT_NEAR(hit.normal.z, 0.0f, EPSILON);

	// OBB rotated 45 degrees around the Y axis
	obb = OBB(
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f))
	);

	ray = Ray(
		glm::vec3(-3.0f, 0.0f, -3.0f),
		glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f))
	);

	hit = Raycast(ray, obb);

	float expectedDistance = sqrtf(18.0f) - 1.0f;
	glm::vec3 expectedPoint = glm::normalize(glm::vec3(-1.0f, 0.0f, -1.0f));
	glm::vec3 expectedNormal = expectedPoint;

	EXPECT_TRUE(hit.didHit);
	EXPECT_NEAR(hit.distance, expectedDistance, EPSILON);

	EXPECT_NEAR(hit.point.x, expectedPoint.x, EPSILON);
	EXPECT_NEAR(hit.point.y, expectedPoint.y, EPSILON);
	EXPECT_NEAR(hit.point.z, expectedPoint.z, EPSILON);

	EXPECT_NEAR(hit.normal.x, expectedNormal.x, EPSILON);
	EXPECT_NEAR(hit.normal.y, expectedNormal.y, EPSILON);
	EXPECT_NEAR(hit.normal.z, expectedNormal.z, EPSILON);
}

TEST(IntersectionTest, RaycastOBBMiss)
{

}

TEST(IntersectionTest, RaycastSphereHit)
{

}

TEST(IntersectionTest, RaycastSphereMiss)
{

}

TEST(IntersectionTest, RaycastFrustumHit)
{

}

TEST(IntersectionTest, RaycastFrustumMiss)
{

}

#pragma endregion


#pragma region AABB



#pragma endregion


#pragma region OBB



#pragma endregion


#pragma region Sphere



#pragma endregion


#pragma region Frustum



#pragma endregion
