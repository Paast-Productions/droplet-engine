#include <gtest/gtest.h>
#include <math/bounds/Intersection.hpp>

using namespace Droplet::Math;

namespace
{
	Frustum MakeAxisAlignedFrustum(float halfExtent)
	{
		return Frustum(
			Plane(glm::vec3(1.0f, 0.0f, 0.0f), -halfExtent),
			Plane(glm::vec3(-1.0f, 0.0f, 0.0f), -halfExtent),
			Plane(glm::vec3(0.0f, -1.0f, 0.0f), -halfExtent),
			Plane(glm::vec3(0.0f, 1.0f, 0.0f), -halfExtent),
			Plane(glm::vec3(0.0f, 0.0f, 1.0f), -halfExtent),
			Plane(glm::vec3(0.0f, 0.0f, -1.0f), -halfExtent)
		);
	}
}

TEST(IntersectionTest, RaycastAabbHitAndMiss)
{
	const AABB aabb(glm::vec3(0.0f), glm::vec3(1.0f));

	const RayHit hit = Raycast(Ray(glm::vec3(-2.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)), aabb);
	EXPECT_TRUE(hit.didHit);
	EXPECT_NEAR(hit.distance, 1.0f, 1e-5f);
	EXPECT_NEAR(hit.hitPoint.x, -1.0f, 1e-5f);
	EXPECT_NEAR(hit.hitNormal.x, -1.0f, 1e-5f);

	const RayHit miss = Raycast(Ray(glm::vec3(-2.0f, 2.5f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)), aabb);
	EXPECT_FALSE(miss.didHit);
}

TEST(IntersectionTest, ContainsPointQueries)
{
	const AABB aabb(glm::vec3(0.0f), glm::vec3(1.0f));
	const OBB obb(glm::vec3(0.0f), glm::vec3(1.0f), glm::mat3(1.0f));
	const Sphere sphere(glm::vec3(0.0f), 1.0f);
	const Frustum frustum = MakeAxisAlignedFrustum(1.0f);

	EXPECT_TRUE(Contains(aabb, glm::vec3(0.5f, 0.0f, 0.0f)));
	EXPECT_FALSE(Contains(aabb, glm::vec3(2.0f, 0.0f, 0.0f)));

	EXPECT_TRUE(Contains(obb, glm::vec3(0.0f, -0.5f, 0.0f)));
	EXPECT_FALSE(Contains(obb, glm::vec3(0.0f, -1.5f, 0.0f)));

	EXPECT_TRUE(Contains(sphere, glm::vec3(0.0f, 0.5f, 0.0f)));
	EXPECT_FALSE(Contains(sphere, glm::vec3(0.0f, 1.5f, 0.0f)));

	EXPECT_TRUE(Contains(frustum, glm::vec3(0.0f, 0.0f, 0.0f)));
	EXPECT_FALSE(Contains(frustum, glm::vec3(2.0f, 0.0f, 0.0f)));
}

TEST(IntersectionTest, AabbAndSphereContainmentClassification)
{
	const AABB largeAabb(glm::vec3(0.0f), glm::vec3(2.0f));
	const AABB smallAabb(glm::vec3(0.0f), glm::vec3(0.5f));
	const AABB farAabb(glm::vec3(5.0f), glm::vec3(0.5f));

	EXPECT_EQ(Intersects(largeAabb, smallAabb), IntersectType::Contains);
	EXPECT_EQ(Intersects(largeAabb, farAabb), IntersectType::None);

	const Sphere largeSphere(glm::vec3(0.0f), 3.0f);
	const Sphere mediumSphere(glm::vec3(0.5f, 0.0f, 0.0f), 1.0f);
	const Sphere farSphere(glm::vec3(7.0f, 0.0f, 0.0f), 1.0f);

	EXPECT_EQ(Intersects(largeSphere, mediumSphere), IntersectType::Contains);
	EXPECT_EQ(Intersects(mediumSphere, farSphere), IntersectType::None);
	EXPECT_EQ(Intersects(mediumSphere, largeSphere), IntersectType::Intersects);
}

TEST(IntersectionTest, FrustumAndSphereClassification)
{
	const Frustum frustum = MakeAxisAlignedFrustum(1.0f);
	const Sphere insideSphere(glm::vec3(0.0f), 0.4f);
	const Sphere crossingSphere(glm::vec3(0.8f, 0.0f, 0.0f), 0.5f);
	const Sphere outsideSphere(glm::vec3(3.0f, 0.0f, 0.0f), 0.5f);
	const Sphere containingSphere(glm::vec3(0.0f), 5.0f);

	EXPECT_EQ(Intersects(frustum, insideSphere), IntersectType::Contains);
	EXPECT_EQ(Intersects(frustum, crossingSphere), IntersectType::Intersects);
	EXPECT_EQ(Intersects(frustum, outsideSphere), IntersectType::None);

	EXPECT_EQ(Intersects(containingSphere, frustum), IntersectType::Contains);
}
