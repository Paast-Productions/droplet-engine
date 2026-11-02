#pragma once
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"

typedef int ImGuiCurveEditFlags;

/// @brief Flags for controlling the behavior and appearance of the curve editor
enum ImGuiCurveEditFlags_
{
	ImGuiCurveEditFlags_None = 0,

	// Shape flags
	ImGuiCurveEditFlags_Linear = 1 << 0,			// Use linear interpolation between points instead of Bezier curves (control points are ignored)
	ImGuiCurveEditFlags_Quadratic = 1 << 1,			// Use quadratic Bezier curves instead of cubic (only one control point per segment, controlPoint1 is used and controlPoint2 is ignored)
	ImGuiCurveEditFlags_Jointed = 1 << 2,			// Control points are automatically mirrored around the main point when editing
	ImGuiCurveEditFlags_ForceSpanWidth = 1 << 3,	// Ensure line spans the entire bound width, by setting x-value of first and last points to bounds min and max
	ImGuiCurveEditFlags_ForceInjective = 1 << 4,	// Ensure curve has exactly one y-value for each x-value

	// UI Interaction flags
	ImGuiCurveEditFlags_ReadOnly = 1 << 5,			// Disable editing of points and control points, but still display them
	ImGuiCurveEditFlags_NoLabels = 1 << 6,			// Disable point number labels (drawn near each point) for better visibility when there are many points
	ImGuiCurveEditFlags_NoPoints = 1 << 7,			// Don't draw the main points
	ImGuiCurveEditFlags_ClampX = 1 << 8,			// Clamp points to bounds on x-axis while editing
	ImGuiCurveEditFlags_ClampY = 1 << 9,			// Clamp points to bounds on y-axis while editing

	// Convenience flags
	ImGuiCurveEditFlags_ClampPoints = ImGuiCurveEditFlags_ClampX | ImGuiCurveEditFlags_ClampY,	// Clamp points to bounds on both axes while editing
};

namespace ImGui
{
	/// @brief A point on a Bezier curve, including its position and control points
	struct BezierPoint
	{
		ImVec2 position = { };
		ImVec2 controlPoint1 = { };
		ImVec2 controlPoint2 = { };
	};

	/// @brief Edit a curve defined by a set of Bezier points
	/// @param label The label for the curve editor
	/// @param points A pointer to a vector of Bezier points defining the curve
	/// @param size The size of the curve editor
	/// @param pointBounds The bounds for the points
	/// @param thickness The thickness of the curve
	/// @param padding The padding around the curve editor
	/// @param gridLines The number of grid lines in the x and y directions
	/// @param flags Flags to control the behavior and appearance of the curve editor
	/// @param childFlags Flags for the child window
	/// @param windowFlags Flags for the window
	/// @return True if the curve was modified, false otherwise
	bool CurveEdit(
		const char* label, 
		std::vector<BezierPoint>* points, 
		ImVec2 size = { 0, 150 }, 
		ImRect pointBounds = { 0, 0, 1, 1 }, 
		float thickness = 2, 
		ImRect padding = { 48, 24, 20, 24 }, 
		ImVec2i gridLines = { 4, 4 }, 
		ImGuiCurveEditFlags flags = 0, 
		ImGuiChildFlags childFlags = 1, 
		ImGuiWindowFlags windowFlags = 0
	);
};
