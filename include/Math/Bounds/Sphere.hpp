#pragma once
#include <glm/glm.hpp>

namespace Droplet::Math
{
	/// @brief Represents a sphere in 3D space with a center and radius.
	struct Sphere
	{
		glm::vec3 center;
		float radius;

		/// @brief Default constructor initializes the sphere with a center at the origin and a radius of zero.
		Sphere() : center(glm::vec3(0.0f)), radius(0.0f) {}

		/// @brief Constructs a sphere with the specified center and radius.
		/// @param p_center The center of the sphere as a glm::vec3.
		/// @param p_radius The radius of the sphere as a float.
		Sphere(const glm::vec3 &p_center, float p_radius) : center(p_center), radius(p_radius) {}

		/// @brief Gets the center of the sphere.
		/// @return The center of the sphere as a glm::vec3.
		[[nodiscard]] inline glm::vec3 GetCenter() const
		{
			return center;
		}

		/// @brief Gets the radius of the sphere.
		/// @return The radius of the sphere as a float.
		[[nodiscard]] inline float GetRadius() const
		{
			return radius;
		}

		[[nodiscard]] static inline Sphere Transform(const Sphere &p_sphere, const glm::mat4 &p_transform)
		{
			glm::vec3 newCenter = glm::vec3(p_transform * glm::vec4(p_sphere.center, 1.0f));
			float newRadius = p_sphere.radius * glm::length(glm::vec3(p_transform[0])); // Assuming uniform scaling
			return Sphere(newCenter, newRadius);
		}
	};
}
