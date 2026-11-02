#include <gtest/gtest.h>
#include <math/bounds/Intersection.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace Droplet::Math;
using namespace glm;

namespace
{
	constexpr float EPSILON = 1e-6f;

	Ray MakeBaseRay()
	{
		return Ray(
			vec3(0.0f, 0.0f, 0.0f),
			vec3(0.0f, 0.0f, 1.0f)
		);
	}

	Plane MakeBasePlane()
	{
		return Plane(
			vec3(0.0f, 1.0f, 0.0f),
			0.0f
		);
	}

	AABB MakeBaseAABB()
	{
		return AABB(
			vec3(0.0f, 0.0f, 0.0f),
			vec3(1.0f, 1.0f, 1.0f)
		);
	}

	OBB MakeBaseOBB()
	{
		return OBB(
			vec3(0.0f, 0.0f, 0.0f),
			vec3(1.0f, 1.0f, 1.0f),
			quat(1.0f, 0.0f, 0.0f, 0.0f)
		);
	}

	Sphere MakeBaseSphere()
	{
		return Sphere(
			vec3(0.0f, 0.0f, 0.0f),
			1.0f
		);
	}

	Frustum MakeBaseFrustum()
	{
		return Frustum(
			Plane(vec3(1.0f, 0.0f, 0.0f), 1.0f),
			Plane(vec3(-1.0f, 0.0f, 0.0f), 1.0f),
			Plane(vec3(0.0f, 1.0f, 0.0f), 1.0f),
			Plane(vec3(0.0f, -1.0f, 0.0f), 1.0f),
			Plane(vec3(0.0f, 0.0f, 1.0f), 1.0f),
			Plane(vec3(0.0f, 0.0f, -1.0f), 1.0f)
		);
	}

	Frustum MakeCameraFrustum(float p_fov, float p_aspect, float p_near, float p_far)
	{
		return Frustum(
			vec3(0.0f, 0.0f, 0.0f),
			vec3(0.0f, 0.0f, 1.0f),
			vec3(0.0f, 1.0f, 0.0f),
			p_fov, p_aspect, p_near, p_far
		);
	}

	Frustum MakeInwardFrustum(Frustum p_frustum)
	{
		for (int i = 0; i < 6; ++i)
		{
			Plane &plane = p_frustum.GetPlane(i);
			plane.normal = -plane.normal;
			plane.distance = -plane.distance;
		}

		return p_frustum;
	}
}


#pragma region Raycasting

TEST(IntersectionTest, RaycastPlaneHit)
{
	Plane plane = MakeBasePlane();

	Ray ray = Ray(
		vec3(0.0f, 1.0f, 0.0f),
		normalize(vec3(0.0f, -1.0f, 1.0f))
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
		vec3(0.0f, 1.0f, 0.0f),
		vec3(0.0f, 1.0f, 0.0f)
	);

	RayHit hitAway = Raycast(rayAway, plane);

	EXPECT_FALSE(hitAway.didHit);

	Ray rayParallel = Ray(
		vec3(0.0f, 1.0f, 0.0f),
		vec3(0.0f, 0.0f, 1.0f)
	);

	RayHit hitParallel = Raycast(rayParallel, plane);

	EXPECT_FALSE(hitParallel.didHit);
}

TEST(IntersectionTest, RaycastAABBHit)
{
	AABB aabb = MakeBaseAABB();

	Ray ray = Ray(
		vec3(-3.0f, 0.0f, 0.0f),
		vec3(1.0f, 0.0f, 0.0f)
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
		vec3(-3.0f, 0.0f, 0.0f),
		normalize(vec3(1.0f, -1.0f, 0.0f))
	);

	RayHit hit = Raycast(ray, aabb);

	EXPECT_FALSE(hit.didHit);
}

TEST(IntersectionTest, RaycastOBBHit)
{
	// Base OBB
	OBB obb = MakeBaseOBB();

	Ray ray = Ray(
		vec3(-3.0f, 0.0f, 0.0f),
		vec3(1.0f, 0.0f, 0.0f)
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
		vec3(0.0f, 0.0f, 0.0f),
		vec3(1.0f, 1.0f, 1.0f),
		angleAxis(radians(45.0f), vec3(0.0f, 1.0f, 0.0f))
	);

	ray = Ray(
		vec3(-3.0f, 0.0f, -3.0f),
		normalize(vec3(1.0f, 0.0f, 1.0f))
	);

	hit = Raycast(ray, obb);

	float expectedDistance = sqrtf(18.0f) - 1.0f;
	vec3 expectedPoint = normalize(vec3(-1.0f, 0.0f, -1.0f));
	vec3 expectedNormal = expectedPoint;

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
	OBB obb = MakeBaseOBB();

	Ray ray = Ray(
		vec3(-3.0f, 0.0f, 0.0f),
		normalize(vec3(1.0f, 1.0f, 0.0f))
	);

	RayHit hit = Raycast(ray, obb);

	EXPECT_FALSE(hit.didHit);
}

TEST(IntersectionTest, RaycastSphereHit)
{
	Sphere sphere = MakeBaseSphere();

	Ray ray = Ray(
		vec3(0.0f, 0.0f, -3.0f),
		vec3(0.0f, 0.0f, 1.0f)
	);

	RayHit hit = Raycast(ray, sphere);

	EXPECT_TRUE(hit.didHit);
	EXPECT_NEAR(hit.distance, 2.0f, EPSILON);

	EXPECT_NEAR(hit.point.x, 0.0f, EPSILON);
	EXPECT_NEAR(hit.point.y, 0.0f, EPSILON);
	EXPECT_NEAR(hit.point.z, -1.0f, EPSILON);

	EXPECT_NEAR(hit.normal.x, 0.0f, EPSILON);
	EXPECT_NEAR(hit.normal.y, 0.0f, EPSILON);
	EXPECT_NEAR(hit.normal.z, -1.0f, EPSILON);
}

TEST(IntersectionTest, RaycastSphereMiss)
{
	Sphere sphere = MakeBaseSphere();

	Ray ray = Ray(
		vec3(0.0f, 0.0f, -3.0f),
		normalize(vec3(1.0f, 1.0f, 1.0f))
	);

	RayHit hit = Raycast(ray, sphere);

	EXPECT_FALSE(hit.didHit);
}

TEST(IntersectionTest, RaycastFrustumHit)
{
	Frustum frustum = MakeBaseFrustum();

	Ray ray = Ray(
		vec3(0.0f, 0.0f, -3.0f),
		vec3(0.0f, 0.0f, 1.0f)
	);

	RayHit hit = Raycast(ray, frustum);

	EXPECT_TRUE(hit.didHit);
	EXPECT_NEAR(hit.distance, 2.0f, EPSILON);

	EXPECT_NEAR(hit.point.x, 0.0f, EPSILON);
	EXPECT_NEAR(hit.point.y, 0.0f, EPSILON);
	EXPECT_NEAR(hit.point.z, -1.0f, EPSILON);

	EXPECT_NEAR(hit.normal.x, 0.0f, EPSILON);
	EXPECT_NEAR(hit.normal.y, 0.0f, EPSILON);
	EXPECT_NEAR(hit.normal.z, -1.0f, EPSILON);

	frustum = MakeCameraFrustum(radians(90.0f), 1.0f, 1.0f, 10.0f);

	ray = Ray(
		vec3(5.0f, 0.0f, 0.0f),
		vec3(0.0f, 0.0f, 1.0f)
	);

	float expectedDistance = 5.0f;
	vec3 expectedPoint = vec3(5.0f, 0.0f, expectedDistance);
	vec3 expectedNormal = normalize(vec3(1.0f, 0.0f, -1.0f));

	hit = Raycast(ray, frustum);

	EXPECT_TRUE(hit.didHit);
	EXPECT_NEAR(hit.distance, expectedDistance, EPSILON);

	EXPECT_NEAR(hit.point.x, expectedPoint.x, EPSILON);
	EXPECT_NEAR(hit.point.y, expectedPoint.y, EPSILON);
	EXPECT_NEAR(hit.point.z, expectedPoint.z, EPSILON);

	EXPECT_NEAR(hit.normal.x, expectedNormal.x, EPSILON);
	EXPECT_NEAR(hit.normal.y, expectedNormal.y, EPSILON);
	EXPECT_NEAR(hit.normal.z, expectedNormal.z, EPSILON);
}

TEST(IntersectionTest, RaycastFrustumMiss)
{
	Frustum frustum = MakeBaseFrustum();

	Ray ray = Ray(
		vec3(0.0f, 0.0f, -3.0f),
		vec3(1.0f, 1.0f, 1.0f)
	);

	RayHit hit = Raycast(ray, frustum);

	EXPECT_FALSE(hit.didHit);

	frustum = MakeCameraFrustum(radians(90.0f), 1.0f, 3.0f, 10.0f);

	ray = Ray(
		vec3(1.65162f, 3.33028f, 0.0f),
		normalize(vec3(-0.4144f, -0.786f, 0.32565f))
	);

	hit = Raycast(ray, frustum);

	EXPECT_FALSE(hit.didHit);
}

#pragma endregion


#pragma region AABB

TEST(IntersectionTest, AABBContainPoint)
{
	AABB aabb = MakeBaseAABB();
	EXPECT_TRUE(Contains(aabb, vec3(0.0f, 0.0f, 0.0f)));	// Center point of the AABB
	EXPECT_FALSE(Contains(aabb, vec3(2.0f, 0.0f, 0.0f)));	// Point outside the AABB on the right

	aabb = AABB(vec3(1, 1, 1), vec3(2, 2, 2));
	EXPECT_TRUE(Contains(aabb, vec3(1.0f, 1.0f, 1.0f)));	// Center point of the AABB
	EXPECT_FALSE(Contains(aabb, vec3(1.0f, 4.0f, 1.0f)));	// Point above the AABB
}

TEST(IntersectionTest, AABBIntersectsAABB)
{
	AABB aabb = MakeBaseAABB();

	EXPECT_EQ(Intersects(aabb, AABB(vec3(0.0f), vec3(0.5f))), IntersectType::Contains);
	EXPECT_EQ(Intersects(aabb, AABB(vec3(1.5f, 0.0f, 0.0f), vec3(1.0f))), IntersectType::Intersects);
	EXPECT_EQ(Intersects(aabb, AABB(vec3(3.0f, 0.0f, 0.0f), vec3(0.5f))), IntersectType::None);
}

TEST(IntersectionTest, AABBIntersectsOBB)
{
	AABB aabb(vec3(0.0f), vec3(2.0f));
	OBB obb(
		vec3(0.0f),
		vec3(0.5f),
		angleAxis(radians(45.0f), vec3(0.0f, 1.0f, 0.0f))
	);

	EXPECT_EQ(Intersects(aabb, obb), IntersectType::Contains);
	EXPECT_EQ(Intersects(MakeBaseAABB(), OBB(vec3(1.5f, 0.0f, 0.0f), vec3(1.0f), quat(1.0f, 0.0f, 0.0f, 0.0f))), IntersectType::Intersects);
	EXPECT_EQ(Intersects(MakeBaseAABB(), OBB(vec3(3.0f, 0.0f, 0.0f), vec3(0.5f), quat(1.0f, 0.0f, 0.0f, 0.0f))), IntersectType::None);
}

TEST(IntersectionTest, AABBIntersectsSphere)
{
	AABB aabb = MakeBaseAABB();

	EXPECT_EQ(Intersects(aabb, Sphere(vec3(0.0f), 0.5f)), IntersectType::Contains);
	EXPECT_EQ(Intersects(aabb, Sphere(vec3(1.5f, 0.0f, 0.0f), 0.75f)), IntersectType::Intersects);
	EXPECT_EQ(Intersects(aabb, Sphere(vec3(3.0f, 0.0f, 0.0f), 0.5f)), IntersectType::None);
}

TEST(IntersectionTest, AABBIntersectsFrustum)
{
	Frustum frustum = MakeInwardFrustum(MakeBaseFrustum());

	EXPECT_EQ(Intersects(AABB(vec3(0.0f), vec3(2.0f)), frustum), IntersectType::Contains);
	EXPECT_EQ(Intersects(AABB(vec3(1.5f, 0.0f, 0.0f), vec3(1.0f)), frustum), IntersectType::Intersects);
	EXPECT_EQ(Intersects(AABB(vec3(3.0f, 0.0f, 0.0f), vec3(0.5f)), frustum), IntersectType::None);

	Frustum cameraFrustum = MakeInwardFrustum(MakeCameraFrustum(radians(90.0f), 1.0f, 1.0f, 10.0f));
	EXPECT_EQ(Intersects(AABB(vec3(0.0f), vec3(11.0f)), cameraFrustum), IntersectType::Contains);
}

#pragma endregion


#pragma region OBB

TEST(IntersectionTest, OBBContainPoint)
{
	OBB obb = MakeBaseOBB();
	EXPECT_TRUE(Contains(obb, vec3(0.0f)));
	EXPECT_FALSE(Contains(obb, vec3(2.0f, 0.0f, 0.0f)));

	quat orientation = angleAxis(radians(45.0f), vec3(0.0f, 1.0f, 0.0f));
	obb = OBB(vec3(0.0f), vec3(2.0f, 1.0f, 1.0f), orientation);
	EXPECT_TRUE(Contains(obb, orientation * vec3(1.0f, 0.0f, 0.0f)));
	EXPECT_FALSE(Contains(obb, orientation * vec3(3.0f, 0.0f, 0.0f)));
}

TEST(IntersectionTest, OBBIntersectsAABB)
{
	OBB obb = OBB(vec3(0.0f), vec3(2.0f), angleAxis(radians(45.0f), vec3(0.0f, 1.0f, 0.0f)));

	EXPECT_EQ(Intersects(obb, AABB(vec3(0.0f), vec3(0.5f))), IntersectType::Contains);
	EXPECT_EQ(Intersects(MakeBaseOBB(), AABB(vec3(1.5f, 0.0f, 0.0f), vec3(1.0f))), IntersectType::Intersects);
	EXPECT_EQ(Intersects(MakeBaseOBB(), AABB(vec3(3.0f, 0.0f, 0.0f), vec3(0.5f))), IntersectType::None);
}

TEST(IntersectionTest, OBBIntersectsOBB)
{
	OBB obb = MakeBaseOBB();

	EXPECT_EQ(Intersects(obb, OBB(vec3(0.0f), vec3(0.5f), quat(1.0f, 0.0f, 0.0f, 0.0f))), IntersectType::Contains);
	EXPECT_EQ(Intersects(obb, OBB(vec3(1.5f, 0.0f, 0.0f), vec3(1.0f), quat(1.0f, 0.0f, 0.0f, 0.0f))), IntersectType::Intersects);
	EXPECT_EQ(Intersects(obb, OBB(vec3(3.0f, 0.0f, 0.0f), vec3(0.5f), quat(1.0f, 0.0f, 0.0f, 0.0f))), IntersectType::None);

	OBB rotatedOBB(vec3(0.0f), vec3(0.5f), angleAxis(radians(45.0f), vec3(0.0f, 1.0f, 0.0f)));
	EXPECT_EQ(Intersects(OBB(vec3(0.0f), vec3(2.0f), quat(1.0f, 0.0f, 0.0f, 0.0f)), rotatedOBB), IntersectType::Contains);
}

TEST(IntersectionTest, OBBIntersectsSphere)
{
	OBB obb = MakeBaseOBB();

	EXPECT_EQ(Intersects(obb, Sphere(vec3(0.0f), 0.5f)), IntersectType::Contains);
	EXPECT_EQ(Intersects(obb, Sphere(vec3(1.5f, 0.0f, 0.0f), 0.75f)), IntersectType::Intersects);
	EXPECT_EQ(Intersects(obb, Sphere(vec3(3.0f, 0.0f, 0.0f), 0.5f)), IntersectType::None);

	obb = OBB(vec3(0.0f), vec3(2.0f), angleAxis(radians(45.0f), vec3(0.0f, 1.0f, 0.0f)));
	EXPECT_EQ(Intersects(obb, Sphere(vec3(0.0f), 0.5f)), IntersectType::Contains);
}

TEST(IntersectionTest, OBBIntersectsFrustum)
{
	Frustum frustum = MakeInwardFrustum(MakeBaseFrustum());

	EXPECT_EQ(Intersects(OBB(vec3(0.0f), vec3(2.0f), quat(1.0f, 0.0f, 0.0f, 0.0f)), frustum), IntersectType::Contains);
	EXPECT_EQ(Intersects(OBB(vec3(1.5f, 0.0f, 0.0f), vec3(1.0f), quat(1.0f, 0.0f, 0.0f, 0.0f)), frustum), IntersectType::Intersects);
	EXPECT_EQ(Intersects(OBB(vec3(3.0f, 0.0f, 0.0f), vec3(0.5f), quat(1.0f, 0.0f, 0.0f, 0.0f)), frustum), IntersectType::None);

	Frustum cameraFrustum = MakeInwardFrustum(MakeCameraFrustum(radians(90.0f), 1.0f, 1.0f, 10.0f));
	EXPECT_EQ(Intersects(OBB(vec3(0.0f, 0.0f, 5.0f), vec3(12.0f), quat(1.0f, 0.0f, 0.0f, 0.0f)), cameraFrustum), IntersectType::Contains);
}

#pragma endregion


#pragma region Sphere

TEST(IntersectionTest, SphereContainPoint)
{
	Sphere sphere = MakeBaseSphere();
	EXPECT_TRUE(Contains(sphere, vec3(0.0f)));
	EXPECT_FALSE(Contains(sphere, vec3(2.0f, 0.0f, 0.0f)));
	EXPECT_TRUE(Contains(sphere, vec3(1.0f - EPSILON, 0.0f, 0.0f)));
}

TEST(IntersectionTest, SphereIntersectsAABB)
{
	Sphere sphere = MakeBaseSphere();

	EXPECT_EQ(Intersects(sphere, AABB(vec3(0.0f), vec3(0.25f))), IntersectType::Contains);
	EXPECT_EQ(Intersects(sphere, MakeBaseAABB()), IntersectType::Intersects);
	EXPECT_EQ(Intersects(sphere, AABB(vec3(3.0f, 0.0f, 0.0f), vec3(0.5f))), IntersectType::None);
}

TEST(IntersectionTest, SphereIntersectsOBB)
{
	Sphere sphere = MakeBaseSphere();
	OBB rotatedOBB(vec3(0.0f), vec3(0.25f), angleAxis(radians(45.0f), vec3(0.0f, 1.0f, 0.0f)));

	EXPECT_EQ(Intersects(sphere, rotatedOBB), IntersectType::Contains);
	EXPECT_EQ(Intersects(sphere, MakeBaseOBB()), IntersectType::Intersects);
	EXPECT_EQ(Intersects(sphere, OBB(vec3(3.0f, 0.0f, 0.0f), vec3(0.5f), quat(1.0f, 0.0f, 0.0f, 0.0f))), IntersectType::None);
}

TEST(IntersectionTest, SphereIntersectsSphere)
{
	Sphere sphere = MakeBaseSphere();

	EXPECT_EQ(Intersects(sphere, Sphere(vec3(0.0f), 0.5f)), IntersectType::Contains);
	EXPECT_EQ(Intersects(sphere, Sphere(vec3(1.5f, 0.0f, 0.0f), 1.0f)), IntersectType::Intersects);
	EXPECT_EQ(Intersects(sphere, Sphere(vec3(3.0f, 0.0f, 0.0f), 1.0f)), IntersectType::None);
}

TEST(IntersectionTest, SphereIntersectsFrustum)
{
	Frustum frustum = MakeInwardFrustum(MakeBaseFrustum());

	EXPECT_EQ(Intersects(Sphere(vec3(0.0f), 2.0f), frustum), IntersectType::Contains);
	EXPECT_EQ(Intersects(MakeBaseSphere(), frustum), IntersectType::Intersects);
	EXPECT_EQ(Intersects(Sphere(vec3(3.0f, 0.0f, 0.0f), 0.5f), frustum), IntersectType::None);

	Frustum cameraFrustum = MakeInwardFrustum(MakeCameraFrustum(radians(90.0f), 1.0f, 1.0f, 10.0f));
	EXPECT_EQ(Intersects(Sphere(vec3(0.0f, 0.0f, 5.0f), 20.0f), cameraFrustum), IntersectType::Contains);
}

#pragma endregion


#pragma region Frustum

TEST(IntersectionTest, FrustumContainsPoint)
{
	Frustum frustum = MakeInwardFrustum(MakeBaseFrustum());

	EXPECT_TRUE(Contains(frustum, vec3(0.0f, 0.0f, 0.0f)));		// Center point of the frustum
	EXPECT_FALSE(Contains(frustum, vec3(2.0f, 0.0f, 0.0f)));	// Point outside the frustum on the right

	frustum = MakeInwardFrustum(MakeCameraFrustum(
		radians(90.0f),
		1.0f,
		1.0f,
		10.0f
	));

	EXPECT_TRUE(Contains(frustum, vec3(0.0f, 0.0f, 5.5f)));		// Frustum center point
	EXPECT_FALSE(Contains(frustum, vec3(6.0f, 0.0f, 5.5f)));	// Point outside the frustum on the right
	EXPECT_FALSE(Contains(frustum, vec3(0.0f, 0.0f, 0.0f)));	// Origin
}

TEST(IntersectionTest, CameraFrustumPlaneNormalsPointOutward)
{
	Frustum frustum = MakeCameraFrustum(radians(90.0f), 1.0f, 1.0f, 10.0f);
	vec3 insidePoint(0.0f, 0.0f, 5.5f);

	for (int i = 0; i < 6; ++i)
	{
		const Plane &plane = frustum.GetPlane(i);
		EXPECT_LT(dot(plane.normal, insidePoint) - plane.distance, -EPSILON);
	}
}

TEST(IntersectionTest, FrustumIntersectsAABB)
{
	Frustum frustum = MakeInwardFrustum(MakeBaseFrustum());

	EXPECT_EQ(Intersects(frustum, AABB(vec3(0.0f), vec3(0.5f))), IntersectType::Contains);
	EXPECT_EQ(Intersects(frustum, AABB(vec3(1.5f, 0.0f, 0.0f), vec3(1.0f))), IntersectType::Intersects);
	EXPECT_EQ(Intersects(frustum, AABB(vec3(3.0f, 0.0f, 0.0f), vec3(0.5f))), IntersectType::None);

	Frustum cameraFrustum = MakeInwardFrustum(MakeCameraFrustum(radians(90.0f), 1.0f, 1.0f, 10.0f));
	EXPECT_EQ(Intersects(cameraFrustum, AABB(vec3(0.0f, 0.0f, 5.0f), vec3(0.25f))), IntersectType::Contains);
}

TEST(IntersectionTest, FrustumIntersectsOBB)
{
	Frustum frustum = MakeInwardFrustum(MakeBaseFrustum());

	EXPECT_EQ(Intersects(frustum, OBB(vec3(0.0f), vec3(0.5f), quat(1.0f, 0.0f, 0.0f, 0.0f))), IntersectType::Contains);
	EXPECT_EQ(Intersects(frustum, OBB(vec3(0.0f), vec3(2.0f), quat(1.0f, 0.0f, 0.0f, 0.0f))), IntersectType::Intersects);
	EXPECT_EQ(Intersects(frustum, OBB(vec3(1.25f, 0.0f, 0.0f), vec3(0.75f), angleAxis(radians(45.0f), vec3(0.0f, 1.0f, 0.0f)))), IntersectType::Intersects);
	EXPECT_EQ(Intersects(frustum, OBB(vec3(3.0f, 0.0f, 0.0f), vec3(0.5f), quat(1.0f, 0.0f, 0.0f, 0.0f))), IntersectType::None);

	Frustum cameraFrustum = MakeInwardFrustum(MakeCameraFrustum(radians(90.0f), 1.0f, 1.0f, 10.0f));
	EXPECT_EQ(Intersects(cameraFrustum, OBB(vec3(0.0f, 0.0f, 5.0f), vec3(0.25f), quat(1.0f, 0.0f, 0.0f, 0.0f))), IntersectType::Contains);
}

TEST(IntersectionTest, FrustumIntersectsSphere)
{
	Frustum frustum = MakeInwardFrustum(MakeBaseFrustum());

	EXPECT_EQ(Intersects(frustum, Sphere(vec3(0.0f), 0.25f)), IntersectType::Contains);
	EXPECT_EQ(Intersects(frustum, Sphere(vec3(0.0f), 2.0f)), IntersectType::Intersects);
	EXPECT_EQ(Intersects(frustum, Sphere(vec3(3.0f, 0.0f, 0.0f), 0.5f)), IntersectType::None);

	Frustum cameraFrustum = MakeInwardFrustum(MakeCameraFrustum(radians(90.0f), 1.0f, 1.0f, 10.0f));
	EXPECT_EQ(Intersects(cameraFrustum, Sphere(vec3(0.0f, 0.0f, 5.0f), 0.25f)), IntersectType::Contains);
}

TEST(IntersectionTest, FrustumIntersectsFrustum)
{
	Frustum frustum = MakeInwardFrustum(MakeBaseFrustum());

	EXPECT_EQ(Intersects(frustum, frustum), IntersectType::Contains);
	EXPECT_EQ(Intersects(frustum, Frustum(
		Plane(vec3(1.0f, 0.0f, 0.0f), -0.5f),
		Plane(vec3(-1.0f, 0.0f, 0.0f), -0.5f),
		Plane(vec3(0.0f, 1.0f, 0.0f), -0.5f),
		Plane(vec3(0.0f, -1.0f, 0.0f), -0.5f),
		Plane(vec3(0.0f, 0.0f, 1.0f), -0.5f),
		Plane(vec3(0.0f, 0.0f, -1.0f), -0.5f)
	)), IntersectType::Contains);
	EXPECT_EQ(Intersects(frustum, Frustum(
		Plane(vec3(1.0f, 0.0f, 0.0f), 0.5f),
		Plane(vec3(-1.0f, 0.0f, 0.0f), -2.5f),
		Plane(vec3(0.0f, 1.0f, 0.0f), -1.0f),
		Plane(vec3(0.0f, -1.0f, 0.0f), -1.0f),
		Plane(vec3(0.0f, 0.0f, 1.0f), -1.0f),
		Plane(vec3(0.0f, 0.0f, -1.0f), -1.0f)
	)), IntersectType::Intersects);
	EXPECT_EQ(Intersects(frustum, Frustum(
		Plane(vec3(1.0f, 0.0f, 0.0f), 3.0f),
		Plane(vec3(-1.0f, 0.0f, 0.0f), -5.0f),
		Plane(vec3(0.0f, 1.0f, 0.0f), -0.5f),
		Plane(vec3(0.0f, -1.0f, 0.0f), -0.5f),
		Plane(vec3(0.0f, 0.0f, 1.0f), -0.5f),
		Plane(vec3(0.0f, 0.0f, -1.0f), -0.5f)
	)), IntersectType::None);

	Frustum cameraFrustum = MakeInwardFrustum(MakeCameraFrustum(radians(90.0f), 1.0f, 1.0f, 10.0f));
	Frustum innerCameraFrustum = MakeInwardFrustum(MakeCameraFrustum(radians(90.0f), 1.0f, 2.0f, 5.0f));
	EXPECT_EQ(Intersects(cameraFrustum, innerCameraFrustum), IntersectType::Contains);
}

#pragma endregion
