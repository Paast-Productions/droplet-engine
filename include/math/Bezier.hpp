//--------------------------------------------------------------------------------------
// File: Bezier.hpp
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// Modified by Paast Productions, 2026.
// Modifications made for use in the Droplet Engine.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
// http://go.microsoft.com/fwlink/?LinkID=615561
//--------------------------------------------------------------------------------------

#pragma once

#include <array>
#include <algorithm>
#include <glm/glm.hpp>

namespace Droplet::Math::Bezier
{
	/// @brief Performs a quadratic bezier interpolation between three control points,
	/// returning the value at the specified time (t ranges 0 to 1).
	/// @param p1 The first control point.
	/// @param p2 The second control point.
	/// @param p3 The third control point.
	/// @param t The parameter value, typically between 0 and 1.
	/// @tparam T The type of the control points, which must support arithmetic operations.
	/// @return The interpolated value at the specified parameter value.
	template<typename T>
	inline T QuadraticInterpolate(T const& p1, T const& p2, T const& p3, float t) noexcept
	{
		return p1 * (1 - t) * (1 - t) +
			p2 * 2 * t * (1 - t) +
			p3 * t * t;
	}

	/// @brief Performs a quadratic bezier interpolation between three control points, returning the value at the specified time (t ranges 0 to 1).
	/// @param p1 The first control point.
	/// @param p2 The second control point.
	/// @param p3 The third control point.
	/// @param t The parameter value, typically between 0 and 1.
	/// @return The interpolated value at the specified parameter value.
	template<>
	inline glm::vec3 QuadraticInterpolate(glm::vec3 const& p1, glm::vec3 const& p2, glm::vec3 const& p3, float t) noexcept
	{
		const glm::vec3 t0 = glm::vec3((1 - t) * (1 - t));
		const glm::vec3 t1 = glm::vec3(2 * t * (1 - t));
		const glm::vec3 t2 = glm::vec3(t * t);

		glm::vec3 result = p1 * t0;
		result = p2 * t1 + result;
		result = p3 * t2 + result;

		return result;
	}


	/// @brief Performs a cubic bezier interpolation between four control points, 
	/// returning the value at the specified time (t ranges 0 to 1).
	/// @param p1 The first control point.
	/// @param p2 The second control point.
	/// @param p3 The third control point.
	/// @param p4 The fourth control point.
	/// @param t The parameter value, typically between 0 and 1.
	/// @tparam T The type of the control points, which must support arithmetic operations.
	/// @return The interpolated value at the specified parameter value.
	template<typename T>
	inline T CubicInterpolate(T const& p1, T const& p2, T const& p3, T const& p4, float t) noexcept
	{
		return p1 * (1 - t) * (1 - t) * (1 - t) +
			p2 * 3 * t * (1 - t) * (1 - t) +
			p3 * 3 * t * t * (1 - t) +
			p4 * t * t * t;
	}
	
	/// @brief Performs a cubic bezier interpolation between four control points, returning the value at the specified time (t ranges 0 to 1).
	/// @param p1 The first control point.
	/// @param p2 The second control point.
	/// @param p3 The third control point.
	/// @param p4 The fourth control point.
	/// @param t The parameter value, typically between 0 and 1.
	/// @return The interpolated value at the specified parameter value.
	template<>
	inline glm::vec3 CubicInterpolate(glm::vec3 const& p1, glm::vec3 const& p2, glm::vec3 const& p3, glm::vec3 const& p4, float t) noexcept
	{
		const glm::vec3 t0 = glm::vec3((1 - t) * (1 - t) * (1 - t));
		const glm::vec3 t1 = glm::vec3(3 * t * (1 - t) * (1 - t));
		const glm::vec3 t2 = glm::vec3(3 * t * t * (1 - t));
		const glm::vec3 t3 = glm::vec3(t * t * t);

		glm::vec3 result = p1 * t0;
		result = p2 * t1 + result;
		result = p3 * t2 + result;
		result = p4 * t3 + result;

		return result;
	}


	/// @brief Computes the tangent of a cubic bezier curve at the specified time.
	/// @param p1 The first control point.
	/// @param p2 The second control point.
	/// @param p3 The third control point.
	/// @param p4 The fourth control point.
	/// @param t The parameter value, typically between 0 and 1.
	/// @tparam T The type of the control points, which must support arithmetic operations.
	/// @return The tangent vector at the specified parameter value.
	template<typename T>
	inline T CubicTangent(T const& p1, T const& p2, T const& p3, T const& p4, float t) noexcept
	{
		return p1 * (-1 + 2 * t - t * t) +
			p2 * (1 - 4 * t + 3 * t * t) +
			p3 * (2 * t - 3 * t * t) +
			p4 * (t * t);
	}

	/// @brief Computes the tangent of a cubic bezier curve at the specified time.
	/// @param p1 The first control point.
	/// @param p2 The second control point.
	/// @param p3 The third control point.
	/// @param p4 The fourth control point.
	/// @param t The parameter value, typically between 0 and 1.
	/// @return The tangent vector at the specified parameter value.
	template<>
	inline glm::vec3 CubicTangent(glm::vec3 const& p1, glm::vec3 const& p2, glm::vec3 const& p3, glm::vec3 const& p4, float t) noexcept
	{
		const glm::vec3 t0 = glm::vec3(-1 + 2 * t - t * t);
		const glm::vec3 t1 = glm::vec3(1 - 4 * t + 3 * t * t);
		const glm::vec3 t2 = glm::vec3(2 * t - 3 * t * t);
		const glm::vec3 t3 = glm::vec3(t * t);

		glm::vec3 result = p1 * t0;
		result = p2 * t1 + result;
		result = p3 * t2 + result;
		result = p4 * t3 + result;

		return result;
	}


	/// @brief Creates vertices for a patch that is tessellated at the specified level.
	/// Calls the specified outputVertex function for each generated vertex,
	/// passing the position, normal, and texture coordinate as parameters.
	/// @param patch An array of 16 control points defining the bezier patch.
	/// @param tessellation The tessellation level of the patch.
	/// @param isMirrored Whether the patch is mirrored, which affects the texture coordinate generation.
	/// @param outputVertex A function that takes three parameters: position (glm::vec3), normal (glm::vec3), and texture coordinate (glm::vec3).
	/// @tparam TOutputFunc The type of the outputVertex function, which must be callable with three parameters: position (glm::vec3), normal (glm::vec3), and texture coordinate (glm::vec3).
	template<typename TOutputFunc>
	void CreatePatchVertices(_In_reads_(16) const glm::vec3 patch[16], std::size_t tessellation, bool isMirrored, TOutputFunc outputVertex)
	{
		for (std::size_t i = 0; i <= tessellation; i++)
		{
			const float u = float(i) / float(tessellation);

			for (std::size_t j = 0; j <= tessellation; j++)
			{
				const float v = float(j) / float(tessellation);

				// Perform four horizontal bezier interpolations
				// between the control points of this patch.
				const glm::vec3 p1 = CubicInterpolate(patch[0], patch[1], patch[2], patch[3], u);
				const glm::vec3 p2 = CubicInterpolate(patch[4], patch[5], patch[6], patch[7], u);
				const glm::vec3 p3 = CubicInterpolate(patch[8], patch[9], patch[10], patch[11], u);
				const glm::vec3 p4 = CubicInterpolate(patch[12], patch[13], patch[14], patch[15], u);

				// Perform a vertical interpolation between the results of the
				// previous horizontal interpolations, to compute the position.
				const glm::vec3 position = CubicInterpolate(p1, p2, p3, p4, v);

				// Perform another four bezier interpolations between the control
				// points, but this time vertically rather than horizontally.
				const glm::vec3 q1 = CubicInterpolate(patch[0], patch[4], patch[8], patch[12], v);
				const glm::vec3 q2 = CubicInterpolate(patch[1], patch[5], patch[9], patch[13], v);
				const glm::vec3 q3 = CubicInterpolate(patch[2], patch[6], patch[10], patch[14], v);
				const glm::vec3 q4 = CubicInterpolate(patch[3], patch[7], patch[11], patch[15], v);

				// Compute vertical and horizontal tangent vectors.
				const glm::vec3 tangent1 = CubicTangent(p1, p2, p3, p4, v);
				const glm::vec3 tangent2 = CubicTangent(q1, q2, q3, q4, u);

				// Cross the two tangent vectors to compute the normal.
				glm::vec3 normal = glm::cross(tangent1, tangent2);

				if (glm::length(normal) > std::numeric_limits<float>::epsilon())
				{
					normal = glm::normalize(normal);

					// If this patch is mirrored, we must invert the normal.
					if (isMirrored)
					{
						normal = -normal;
					}
				}
				else
				{
					// In a tidy and well constructed bezier patch, the preceding
					// normal computation will always work. But the classic teapot
					// model is not tidy or well constructed! At the top and bottom
					// of the teapot, it contains degenerate geometry where a patch
					// has several control points in the same place, which causes
					// the tangent computation to fail and produce a zero normal.
					// We 'fix' these cases by just hard-coding a normal that points
					// either straight up or straight down, depending on whether we
					// are on the top or bottom of the teapot. This is not a robust
					// solution for all possible degenerate bezier patches, but hey,
					// it's good enough to make the teapot work correctly!

					normal = (position.y < 0) ? glm::vec3(0, -1, 0) : glm::vec3(0, 1, 0);
				}

				// Compute the texture coordinate.
				const float mirroredU = isMirrored ? 1 - u : u;

				const glm::vec3 textureCoordinate = glm::vec3(mirroredU, v, 0);

				// Output this vertex.
				outputVertex(position, normal, textureCoordinate);
			}
		}
	}


	/// @brief Creates indices for a patch that is tessellated at the specified level.
	/// Calls the specified outputIndex function for each generated index value.
	/// @param tessellation The tessellation level of the patch.
	/// @param isMirrored Whether the patch is mirrored, which affects the winding order of the indices.
	/// @param outputIndex A function that takes a single std::size_t parameter, which will be called for each generated index value.
	/// @tparam TOutputFunc The type of the outputIndex function, which must be callable with a single std::size_t parameter.
	template<typename TOutputFunc>
	void CreatePatchIndices(std::size_t tessellation, bool isMirrored, TOutputFunc outputIndex)
	{
		std::size_t stride = tessellation + 1;

		for (std::size_t i = 0; i < tessellation; i++)
		{
			for (std::size_t j = 0; j < tessellation; j++)
			{
				// Make a list of six index values (two triangles).
				std::array<std::size_t, 6> indices =
				{
					i * stride + j,
					(i + 1) * stride + j,
					(i + 1) * stride + j + 1,

					i * stride + j,
					(i + 1) * stride + j + 1,
					i * stride + j + 1,
				};

				// If this patch is mirrored, reverse indices to fix the winding order.
				if (isMirrored)
				{
					std::reverse(indices.begin(), indices.end());
				}

				// Output these index values.
				std::for_each(indices.begin(), indices.end(), outputIndex);
			}
		}
	}
}
