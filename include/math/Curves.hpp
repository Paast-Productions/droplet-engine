#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace Droplet::Math::Curves
{
	enum class CurveType
	{
		Linear,
		BezierQuadratic,
		BezierCubic
	};

	/// @brief Determines if a cubic bezier curve is x-monotonic.
	/// @tparam TVec The vector type (e.g., glm::vec2, glm::vec3).
	/// @param p0 The first control point of the cubic bezier curve.
	/// @param p1 The second control point of the cubic bezier curve.
	/// @param p2 The third control point of the cubic bezier curve.
	/// @param p3 The fourth control point of the cubic bezier curve.
	/// @return True if the cubic bezier curve is x-monotonic, false otherwise.
	template<typename TVec>
	bool IsXMonotonic(const TVec &p0, const TVec &p1, const TVec &p2, const TVec &p3);

	/// @brief Represents a point in a curve, including its position and control points.
	/// @tparam TVec The vector type (e.g., glm::vec2, glm::vec3).
	template<typename TVec>
	struct Point
	{
		TVec position = { };
		TVec controlPoint1 = { };
		TVec controlPoint2 = { };
	};

	/// @brief Samples a point on a curve defined by two points and a curve type at a given parameter t.
	/// @tparam TVec The vector type (e.g., glm::vec2, glm::vec3).
	/// @param lP The left point of the curve, including its position and control points.
	/// @param rP The right point of the curve, including its position and control points.
	/// @param type The type of the curve (Linear, BezierQuadratic, BezierCubic).
	/// @param t The parameter along the curve, typically in the range [0, 1].
	/// @return The sampled point on the curve at parameter t.
	template<typename TVec>
	TVec Sample(const Point<TVec> &lP, const Point<TVec> &rP, CurveType type, float t);

	/// @brief Represents a curve defined by a series of points and a curve type.
	/// @tparam TVec The vector type (e.g., glm::vec2, glm::vec3).
	template<typename TVec>
	struct Curve
	{
		std::vector<Point<TVec>> points;
		CurveType type = CurveType::BezierCubic;

		/// @brief Determines if the curve is injective (one-to-one) at a specific point index.
		/// @param i The index of the point in the curve to check for injectivity.
		/// @return True if the curve is injective at the specified point index, false otherwise.
		bool IsInjectiveAtPoint(int i) const;

		/// @brief Determines if the entire curve is injective (one-to-one).
		/// @return True if the entire curve is injective, false otherwise.
		bool IsInjective() const;

		/// @brief Samples a point on the curve at a given parameter t.
		/// @param t The parameter along the curve, typically in the range [0, 1].
		/// @return The sampled point on the curve at parameter t.
		TVec SamplePoint(float t) const;

		/// @brief Samples a point on the curve at a given parameter t, assuming the curve is injective.
		/// @param x The parameter along the curve, typically in the range [0, 1], assuming the curve is injective.
		/// @return The sampled point on the curve at parameter x, assuming the curve is injective.
		float SampleInjective(float x) const;
	};
};
