#include "math/bounds/OBB.hpp"
#include <glm/gtx/pca.hpp>
#include <glm/gtc/quaternion.hpp>

using namespace Droplet::Math;

OBB::OBB(const glm::vec3 &p_center, const glm::vec3 &p_extents, const glm::quat &p_quat) :
	center(p_center), extents(p_extents), orientation(glm::mat3_cast(p_quat)) { }


// HACK: No idea if this function works. Tests must be written to verify it.
OBB OBB::FromPoints(const std::vector<glm::vec3> &p_points)
{
	OBB result{};

	if (p_points.empty())
	{
		return result;
	}

	const size_t n = p_points.size();
	if (n == 1)
	{
		result.center = p_points[0];
		result.extents = glm::vec3(0.0f);
		result.orientation = glm::mat3(1.0f);
		return result;
	}

	// 1. Centroid
	glm::vec3 mean(0.0f);
	for (const auto &p : p_points)
	{
		mean += p;
	}

	mean /= static_cast<float>(n);

	// 2. Covariance matrix (GLM helper)
	//    points are absolute coordinates + pre-computed center
	const glm::mat3 cov = glm::computeCovarianceMatrix(p_points.data(), n, mean);

	// 3. Eigen-decomposition
	glm::vec3 eigenvalues;
	glm::mat3 eigenvectors;
	const unsigned int found = glm::findEigenvaluesSymReal(cov, eigenvalues, eigenvectors);

	if (found != 3)
	{
		// Degenerate case - fall back to identity orientation
		result.center = mean;
		result.extents = glm::vec3(0.0f);
		result.orientation = glm::mat3(1.0f);
		return result;
	}

	// Sort largest -> smallest eigenvalue (optional but conventional for OBB axes)
	glm::sortEigenvalues(eigenvalues, eigenvectors);

	// Guarantee a right-handed orthonormal basis
	eigenvectors[0] = glm::normalize(eigenvectors[0]);
	eigenvectors[1] = glm::normalize(eigenvectors[1]);
	eigenvectors[2] = glm::cross(eigenvectors[0], eigenvectors[1]);
	// Re-orthogonalise second axis against numerical drift
	eigenvectors[1] = glm::cross(eigenvectors[2], eigenvectors[0]);

	// 4. Project every point onto the principal axes -> local AABB
	glm::vec3 localMin(std::numeric_limits<float>::max());
	glm::vec3 localMax(-std::numeric_limits<float>::max());

	for (const auto &p : p_points)
	{
		const glm::vec3 d = p - mean;
		const glm::vec3 local(
			glm::dot(d, eigenvectors[0]),
			glm::dot(d, eigenvectors[1]),
			glm::dot(d, eigenvectors[2]));

		localMin = glm::min(localMin, local);
		localMax = glm::max(localMax, local);
	}

	// 5. Convert the local AABB back to world space
	const glm::vec3 localCenter = 0.5f * (localMin + localMax);
	const glm::vec3 localExtents = 0.5f * (localMax - localMin);

	result.center = mean
		+ eigenvectors[0] * localCenter.x
		+ eigenvectors[1] * localCenter.y
		+ eigenvectors[2] * localCenter.z;

	result.extents = localExtents;
	result.orientation = eigenvectors;   // columns = principal axes

	return result;
}
