#ifdef _EDITOR
#include "ImGuiCurve.hpp"

using namespace ImGui;

static bool IsMouseHoveringLine(const ImVec2& p1, const ImVec2& p2, float thickness)
{
	ImVec2 lineDir = p2 - p1;
	float lineLength = sqrtf(lineDir.x * lineDir.x + lineDir.y * lineDir.y);
	if (lineLength == 0.0f)
		return false;

	lineDir.x /= lineLength;
	lineDir.y /= lineLength;

	ImVec2 mousePos = GetIO().MousePos;
	ImVec2 toMouse = mousePos - p1;

	float t = toMouse.x * lineDir.x + toMouse.y * lineDir.y;
	t = std::max(0.0f, std::min(lineLength, t));

	ImVec2 closestPoint = p1 + ImVec2(lineDir.x * t, lineDir.y * t);
	ImVec2 diff = mousePos - closestPoint;

	float distanceSq = diff.x * diff.x + diff.y * diff.y;
	return distanceSq <= thickness * thickness;
}

static void MoveCP(ImVec2& cp, ImVec2& cpM, const ImVec2& p, const ImVec2& newPos, bool jointed)
{
	cp = newPos;
	if (jointed)
		cpM = p * 2.0f - cp; // Set mirror control point to be symmetric across the main point
}

static bool IsXMonotonic(const ImVec2& p0, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3)
{
	// Standard expanded quadratic for dx/dt:
	float A = -3 * p0.x + 9 * p1.x - 9 * p2.x + 3 * p3.x;
	float B = 6 * p0.x - 12 * p1.x + 6 * p2.x;
	float C = -3 * p0.x + 3 * p1.x;

	// Discriminant
	float disc = B * B - 4 * A * C;
	if (disc < 0)
		return true; // No real roots = quadratic doesn't cross zero

	// Roots
	float sqrtD = sqrtf(disc);
	float t1 = (-B - sqrtD) / (2 * A);
	float t2 = (-B + sqrtD) / (2 * A);

	// Check if any root lies strictly inside (0,1)
	bool hasInteriorRoot = false;

	if (A != 0)
	{
		// Quadratic case
		if (t1 > 0 && t1 < 1) hasInteriorRoot = true;
		if (t2 > 0 && t2 < 1) hasInteriorRoot = true;
	}
	else if (B != 0)
	{
		// Linear case (degenerate)
		float t = -C / B;
		if (t > 0 && t < 1) hasInteriorRoot = true;
	}

	return !hasInteriorRoot;
}

static bool IsInjective(const BezierPoint& lP, const BezierPoint& rP, ImGuiCurveEditFlags flags)
{
	bool linear = (flags & ImGuiCurveEditFlags_Linear) != 0;
	bool quadratic = (flags & ImGuiCurveEditFlags_Quadratic) != 0;

	if (lP.position.x >= rP.position.x)
		return false;

	if (linear)
		return true;

	if (quadratic)
	{
		// For a quadratic Bezier curve defined by points P0, P1, P2, 
		// the curve is non-injective if P1.x is not between P0.x and P2.x.
		ImVec2 p0 = lP.position;
		ImVec2 p1 = lP.controlPoint1;
		ImVec2 p2 = rP.position;

		if (p1.x < p0.x)
			return false;

		if (p1.x > p2.x)
			return false;
	}
	else // Cubic
	{
		// For a cubic Bezier curve defined by points P0, P1, P2, P3,
		// the curve is non-injective if P1.x is less than P0.x or P2.x is greater than P3.x.
		// The curve may be non-injective if P1.x is greater than P3.x or P2.x is less than P0.x, depending on the x-value of the other control point.
		ImVec2 p0 = lP.position;
		ImVec2 p1 = lP.controlPoint2;
		ImVec2 p2 = rP.controlPoint1;
		ImVec2 p3 = rP.position;

		if (p1.x < p0.x)
			return false;

		if (p2.x > p3.x)
			return false;

		if (p1.x > p3.x || p2.x < p0.x)
		{
			// Possibly non-injective

			if (!IsXMonotonic(p0, p1, p2, p3))
				return false;
		}
	}

	return true;
}

static bool IsInjective(const BezierPoint* points, int c, ImGuiCurveEditFlags flags)
{
	if (c <= 1)
		return true;

	for (int i = 1; i < c; i++)
	{
		const BezierPoint& lP = points[i - 1];
		const BezierPoint& rP = points[i];

		if (!IsInjective(lP, rP, flags))
			return false;
	}

	return true;
}

bool ImGui::CurveEdit(const char* label, std::vector<BezierPoint>* points,
	ImVec2 size, ImRect pointBounds, float thickness, ImRect padding, ImVec2i gridLines,
	ImGuiCurveEditFlags flags, ImGuiChildFlags childFlags, ImGuiWindowFlags windowFlags)
{
	bool linear = (flags & ImGuiCurveEditFlags_Linear) != 0;
	bool quadratic = (flags & ImGuiCurveEditFlags_Quadratic) != 0;
	bool jointed = (flags & ImGuiCurveEditFlags_Jointed) != 0;
	bool forceSpanWidth = (flags & ImGuiCurveEditFlags_ForceSpanWidth) != 0;
	bool forceInjective = (flags & ImGuiCurveEditFlags_ForceInjective) != 0;
	bool readOnly = (flags & ImGuiCurveEditFlags_ReadOnly) != 0;
	bool noLabels = (flags & ImGuiCurveEditFlags_NoLabels) != 0;
	bool noPoints = (flags & ImGuiCurveEditFlags_NoPoints) != 0;
	bool clampX = (flags & ImGuiCurveEditFlags_ClampX) != 0;
	bool clampY = (flags & ImGuiCurveEditFlags_ClampY) != 0;

	if (CalcTextSize(label, 0, true).x > 0.0f)
	{
		Text(label);
		SameLine(0.0f, GetStyle().ItemSpacing.x);
	}

	if (size.x <= 0)
	{
		size.x = GetContentRegionAvail().x;
	}

	if (size.y <= 0)
	{
		size.y = GetContentRegionAvail().y;
	}

	BeginChild(label, size, childFlags, windowFlags);
	ImGuiWindow* window = GetCurrentWindow();
	ImGuiStorage* storage = GetStateStorage();
	ImDrawList* drawList = GetWindowDrawList();
	ImGuiIO& io = GetIO();

	if (window->SkipItems)
	{
		EndChild();
		return false;
	}

	if (!points)
	{
		EndChild();
		return false;
	}

	int pointCount = (int)points->size();

	if (pointCount <= 1)
	{
		EndChild();
		return false;
	}

	ImVec2 windowPos = GetWindowPos();
	ImRect drawArea = ImRect(windowPos, windowPos + size);
	ImRect contentArea = ImRect(drawArea.Min + ImVec2(padding.Min.x, padding.Max.y), drawArea.Max - ImVec2(padding.Max.x, padding.Min.y));
	ImVec2 contentSize = contentArea.GetSize();

	ImVec2 pScale = ImVec2(contentArea.GetWidth() / pointBounds.GetWidth(), -contentArea.GetHeight() / pointBounds.GetHeight());
	ImVec2 pOffset = windowPos + ImVec2(padding.Min.x - (pointBounds.Min.x * pScale.x), padding.Max.y + contentArea.GetHeight() - (pointBounds.Min.y * pScale.y));

	drawList->PushClipRect(drawArea.Min, drawArea.Max, true);

	// Draw barrier rect around content area
	drawList->AddRect(contentArea.Min, contentArea.Max, IM_COL32(96, 96, 96, 255), 0.0f, 0, 2.0f);

	// Draw grid lines
	if (gridLines.x > 0)
	{
		for (int i = 0; i <= gridLines.x; i++)
		{
			float t = (float)i / gridLines.x;
			ImVec2 p1 = ImVec2(contentArea.Min.x + t * contentSize.x, contentArea.Min.y);
			ImVec2 p2 = ImVec2(contentArea.Min.x + t * contentSize.x, contentArea.Max.y);
			drawList->AddLine(p1, p2, IM_COL32(80, 80, 80, 255), 1.0f);

			// Label x-value to the bottom of the grid line
			if (!noLabels)
			{
				char label[32]{};
				snprintf(label, 32, "%.1f", pointBounds.Min.x + t * pointBounds.GetWidth());
				ImVec2 textSize = CalcTextSize(label);
				drawList->AddText(ImVec2(p1.x - textSize.x * 0.5f, contentArea.Max.y + 8), IM_COL32(255, 255, 255, 255), label);
			}
		}
	}
	if (gridLines.y > 0)
	{
		for (int i = 0; i <= gridLines.y; i++)
		{
			float t = (float)i / gridLines.y;
			ImVec2 p1 = ImVec2(contentArea.Min.x, contentArea.Min.y + t * contentSize.y);
			ImVec2 p2 = ImVec2(contentArea.Max.x, contentArea.Min.y + t * contentSize.y);
			drawList->AddLine(p1, p2, IM_COL32(80, 80, 80, 255), 1.0f);

			// Label y-value to the left of the grid line
			if (!noLabels)
			{
				char label[32]{};
				snprintf(label, 32, "%.1f", pointBounds.Max.y - t * pointBounds.GetHeight());
				ImVec2 textSize = CalcTextSize(label);
				drawList->AddText(ImVec2(contentArea.Min.x - textSize.x - 8, p1.y - textSize.y * 0.5f), IM_COL32(255, 255, 255, 255), label);
			}
		}
	}

	for (int i = 0; i < pointCount - 1; i++)
	{
		if (linear)
		{
			ImVec2 p1 = (*points)[i].position * pScale + pOffset;
			ImVec2 p2 = (*points)[i + 1ll].position * pScale + pOffset;

			drawList->AddLine(p1, p2, IM_COL32(128, 128, 128, 255), thickness);
		}
		else if (quadratic)
		{
			ImVec2 p1 = (*points)[i].position * pScale + pOffset;
			ImVec2 p2 = (*points)[i].controlPoint1 * pScale + pOffset;
			ImVec2 p3 = (*points)[i + 1ll].position * pScale + pOffset;

			drawList->AddBezierQuadratic(p1, p2, p3, IM_COL32(128, 128, 128, 255), thickness);
		}
		else
		{
			ImVec2 p1 = (*points)[i].position * pScale + pOffset;
			ImVec2 p2 = (*points)[i].controlPoint2 * pScale + pOffset;
			ImVec2 p3 = (*points)[i + 1ll].controlPoint1 * pScale + pOffset;
			ImVec2 p4 = (*points)[i + 1ll].position * pScale + pOffset;

			drawList->AddBezierCubic(p1, p2, p3, p4, IM_COL32(128, 128, 128, 255), thickness);
		}
	}

	bool isDragging = false;
	bool mouseAbsorbed = false;
	bool changed = false;

	if (forceSpanWidth)
	{
		// Ensure first and last points span the entire width of the bounds
		BezierPoint& firstPoint = (*points)[0];
		BezierPoint& lastPoint = (*points)[pointCount - 1ll];

		if (firstPoint.position.x != pointBounds.Min.x)
		{
			float deltaX = pointBounds.Min.x - firstPoint.position.x;
			firstPoint.position.x = pointBounds.Min.x;
			firstPoint.controlPoint1.x += deltaX;
			firstPoint.controlPoint2.x += deltaX;
			changed = true;
		}

		if (lastPoint.position.x != pointBounds.Max.x)
		{
			float deltaX = pointBounds.Max.x - lastPoint.position.x;
			lastPoint.position.x = pointBounds.Max.x;
			lastPoint.controlPoint1.x += deltaX;
			lastPoint.controlPoint2.x += deltaX;
			changed = true;
		}
	}

	if (forceInjective)
	{
		for (int i = 1; i < pointCount; i++)
		{
			BezierPoint& lP = (*points)[i - 1ll];
			BezierPoint& rP = (*points)[i];

			if (lP.position.x >= rP.position.x)
			{
				// Shift the right point to be just to the right of the left point
				float newX = lP.position.x + 1e-6f; // Small epsilon to ensure separation
				float deltaX = newX - rP.position.x;
				rP.position.x = newX;
				rP.controlPoint1.x += deltaX;
				rP.controlPoint2.x += deltaX;
				changed = true;
			}

			if (linear)
				continue;

			if (quadratic)
			{
				// Ensure control point is between the main points
				if (lP.controlPoint1.x < lP.position.x)
				{
					lP.controlPoint1.x = lP.position.x;
					changed = true;
				}
				else if (lP.controlPoint1.x > rP.position.x)
				{
					lP.controlPoint1.x = rP.position.x;
					changed = true;
				}
			}
			else // Cubic
			{
				// Ensure control points are in a valid configuration
				if (lP.controlPoint2.x < lP.position.x)
				{
					lP.controlPoint2.x = lP.position.x;
					changed = true;
				}

				if (rP.controlPoint1.x > rP.position.x)
				{
					rP.controlPoint1.x = rP.position.x;
					changed = true;
				}

				int attempts = 0;
				while (attempts++ < 32) // Prevent infinite loop in extreme cases
				{
					bool monotonic = true;

					if (lP.controlPoint2.x > rP.position.x)
					{
						if (!IsXMonotonic(lP.position, lP.controlPoint1, rP.controlPoint1, rP.position))
						{
							ImVec2 pToCp = lP.controlPoint2 - lP.position;
							pToCp *= 0.995f; // Scale back slightly to ensure monotonicity

							MoveCP(lP.controlPoint2, lP.controlPoint1, lP.position, lP.position + pToCp, jointed);
							changed = true;
							monotonic = false;
						}
					}

					if (rP.controlPoint1.x < lP.position.x)
					{
						attempts = 0;
						if (!IsXMonotonic(lP.position, lP.controlPoint2, rP.controlPoint2, rP.position))
						{
							ImVec2 pToCp = rP.controlPoint1 - rP.position;
							pToCp *= 0.995f; // Scale back slightly to ensure monotonicity

							MoveCP(rP.controlPoint1, rP.controlPoint2, rP.position, rP.position + pToCp, jointed);
							changed = true;
							monotonic = false;
						}
					}

					if (monotonic)
						break;
				}
			}
		}
	}

	bool multiSelect = IsKeyDown(ImGuiMod_Shift);
	bool snapping = IsKeyDown(ImGuiMod_Ctrl);

	static BezierPoint unSnappedPos = BezierPoint();
	static ImVec2 deltaBuffer = ImVec2();

	for (int i = 0; i < pointCount; i++)
	{
		assert(i >= 0 && i < points->size());

		ImVec2 pPos = (*points)[i].position * pScale + pOffset;

		// Handle point selection
		ImGuiID pointID = window->GetID(std::format("##Point{}", i).c_str());
		ImGuiID pointPrevID = window->GetID(std::format("##PointPrev{}", i).c_str());
		ImGuiID pointDragID = window->GetID(std::format("##PointDrag{}", i).c_str());

		int pointSelectState = storage->GetInt(pointID);
		int pointPrevSelectState = storage->GetInt(pointPrevID);
		int pointDragState = storage->GetInt(pointDragID);

		if (!mouseAbsorbed && IsMouseHoveringRect(pPos - ImVec2(thickness, thickness) * 2.5f, pPos + ImVec2(thickness, thickness) * 2.5f))
		{
			SetMouseCursor(ImGuiMouseCursor_Hand);
			if (IsMouseClicked(ImGuiMouseButton_Left))
			{
				storage->SetInt(pointID, 2);
				storage->SetInt(pointPrevID, (pointSelectState > 0) ? 1 : 0);
				storage->SetInt(pointDragID, 0);

				deltaBuffer = ImVec2(0, 0);
				unSnappedPos = (*points)[i];
				mouseAbsorbed = true;
			}
			else if (!readOnly && pointCount > 2 && IsMouseClicked(ImGuiMouseButton_Right))
			{
				// Handle point deletion
				points->erase(points->begin() + i--);
				i--;
				pointCount--;

				storage->Clear();
				changed = true;
				mouseAbsorbed = true;
				continue;
			}
		}

		if (pointSelectState == 2)
		{
			mouseAbsorbed = true;

			if (!IsMouseDown(ImGuiMouseButton_Left))
			{
				if (pointDragState == 0 && !multiSelect)
					storage->Clear(); // Clear other selections

				storage->SetInt(pointID, (pointDragState == 0) ? (1 - pointPrevSelectState) : pointPrevSelectState);
				storage->SetInt(pointPrevID, 0);
			}
			else if (!readOnly)
			{
				// Handle point dragging
				if (IsMouseDragging(ImGuiMouseButton_Left, thickness) && !isDragging)
				{
					storage->SetInt(pointDragID, 1);
					isDragging = true;
					changed = true;

					ImVec2 mouseDelta = io.MouseDelta;
					ImVec2 delta = mouseDelta / pScale; // Convert back to curve space

					// Delta must be used to neutralize buffer before the point can move again
					// This keeps the cursor following to the point even when returning from out of bounds
					if (deltaBuffer.x != 0 || deltaBuffer.y != 0)
					{
						if (deltaBuffer.x != 0 && delta.x != 0 && std::signbit(deltaBuffer.x) != std::signbit(delta.x))
						{
							if (fabsf(delta.x) > fabsf(deltaBuffer.x))
							{
								delta.x += deltaBuffer.x;
								deltaBuffer.x = 0;
							}
							else
							{
								deltaBuffer.x += delta.x;
								delta.x = 0;
							}
						}

						if (deltaBuffer.y != 0 && delta.y != 0 && std::signbit(deltaBuffer.y) != std::signbit(delta.y))
						{
							if (fabsf(delta.y) > fabsf(deltaBuffer.y))
							{
								delta.y += deltaBuffer.y;
								deltaBuffer.y = 0;
							}
							else
							{
								deltaBuffer.y += delta.y;
								delta.y = 0;
							}
						}
					}

					if (clampX)
					{
						float newPosX = unSnappedPos.position.x + delta.x;
						newPosX = std::clamp(newPosX, pointBounds.Min.x, pointBounds.Max.x);

						float newDeltaX = newPosX - unSnappedPos.position.x;
						deltaBuffer.x += (delta.x - newDeltaX); // Track how much delta was negated
						delta.x = newDeltaX;
					}

					if (clampY)
					{
						float newPosY = unSnappedPos.position.y + delta.y;
						newPosY = std::clamp(newPosY, pointBounds.Min.y, pointBounds.Max.y);

						float newDeltaY = newPosY - unSnappedPos.position.y;
						deltaBuffer.y += (delta.y - newDeltaY); // Track how much delta was negated
						delta.y = newDeltaY;
					}

					unSnappedPos.position += delta;
					unSnappedPos.controlPoint1 += delta;
					unSnappedPos.controlPoint2 += delta;

					if (snapping)
					{
						// Set point pos to nearest grid intersection
						ImVec2 pToCp1 = unSnappedPos.controlPoint1 - unSnappedPos.position;
						ImVec2 pToCp2 = unSnappedPos.controlPoint2 - unSnappedPos.position;

						ImVec2 snappedPos;
						snappedPos.x = roundf(unSnappedPos.position.x / pointBounds.GetWidth() * gridLines.x) * (pointBounds.GetWidth() / gridLines.x);
						snappedPos.y = roundf(unSnappedPos.position.y / pointBounds.GetHeight() * gridLines.y) * (pointBounds.GetHeight() / gridLines.y);

						(*points)[i].position = snappedPos;
						(*points)[i].controlPoint1 = snappedPos + pToCp1;
						(*points)[i].controlPoint2 = snappedPos + pToCp2;
					}
					else
					{
						(*points)[i] = unSnappedPos;
					}

					if (!noLabels)
					{
						// While dragging, draw vector coords to bottom right
						char label[64]{};
						snprintf(label, 64, "X%.3f\nY%.3f", (*points)[i].position.x, (*points)[i].position.y);
						ImVec2 textSize = CalcTextSize(label);
						drawList->AddText(ImVec2(contentArea.Max.x - textSize.x - 8, contentArea.Max.y - textSize.y - 8), IM_COL32(255, 255, 255, 255), label);
					}
				}
			}
		}

		if (!linear && (pointSelectState == 1 || pointPrevSelectState == 1))
		{
			for (int j = 0; j < 2; j++)
			{
				if (quadratic)
				{
					// For quadratic curves, skip cp2 for all points and cp1 for the last point
					if (j == 1)
						continue;
					if (i == pointCount - 1)
						continue;
				}
				else
				{
					// For cubic curved, skip cp1 for the first point and cp2 for the last point
					if (j == 0 && i == 0)
						continue;
					if (j == 1 && i == pointCount - 1)
						continue;
				}

				std::string cpLabel = std::format("##CP{}:{}", j + 1, i);
				ImVec2* cp = (j == 0) ? (&(*points)[i].controlPoint1) : (&(*points)[i].controlPoint2);
				ImVec2* cpMirror = (j == 0) ? (&(*points)[i].controlPoint2) : (&(*points)[i].controlPoint1);
				ImColor cpColor = (j == 0) ? ImColor(255, 64, 64, 255) : ImColor(64, 64, 255, 255);

				// If the control point is too close to the main point, add a visual offset to make it easier to select
				ImVec2 unOverlapOffset = ImVec2(0, 0);
				float distSq = ImLengthSqr((*cp) - (*points)[i].position);

				float overlapThreshold = 0.005f * thickness * (pointBounds.GetWidth() + pointBounds.GetHeight());
				float overlapThresholdSqr = overlapThreshold * overlapThreshold;

				if (distSq < overlapThresholdSqr)
				{
					ImVec2 dir{};
					if (distSq < 1e-6)
					{
						dir = ImVec2((quadratic || j != 0) ? 1 : -1, 0);
					}
					else
					{
						float dist = sqrtf(distSq);
						dir = ((*cp) - (*points)[i].position) * (1.0f / dist);
					}

					unOverlapOffset = dir * overlapThreshold;
					unOverlapOffset += (*points)[i].position - (*cp);
				}

				ImVec2 cpPos = ((*cp) + unOverlapOffset) * pScale + pOffset;
				static ImVec2 unSnappedCpPos = ImVec2(0, 0);

				drawList->AddLine(cpPos, pPos, IM_COL32(255, 255, 255, 255), thickness * 0.5f);
				drawList->AddCircleFilled(cpPos + unOverlapOffset, thickness, cpColor);

				// Handle control point dragging
				ImGuiID cpID = window->GetID(cpLabel.c_str());

				int cpState = storage->GetInt(cpID);
				bool isCPSelected = cpState == 1;

				if (!mouseAbsorbed)
				{
					bool isHoveringLine = IsMouseHoveringLine(pPos, cpPos, thickness);
					bool isHoveringRect = IsMouseHoveringRect(cpPos - ImVec2(thickness, thickness) * 2.0f, cpPos + ImVec2(thickness, thickness) * 2.0f);

					if (isHoveringLine || isHoveringRect)
					{
						SetMouseCursor(ImGuiMouseCursor_Hand);
						if (IsMouseClicked(ImGuiMouseButton_Left))
						{
							if (!readOnly && isHoveringLine && !isHoveringRect)
							{
								// Snap control point to mouse when clicking on line
								ImVec2 mousePos = io.MousePos;
								mousePos = (mousePos - pOffset) / pScale; // Convert to curve space

								MoveCP(*cp, *cpMirror, (*points)[i].position, mousePos, jointed);
								changed = true;
							}

							storage->SetInt(cpID, isCPSelected ? 0 : 1);
							unSnappedCpPos = *cp;
						}

						mouseAbsorbed = true;
					}
				}

				if (isCPSelected)
				{
					if (!IsMouseDown(ImGuiMouseButton_Left))
					{
						storage->SetInt(cpID, 0);
					}
					else if (!readOnly && IsMouseDragging(ImGuiMouseButton_Left, thickness))
					{
						ImVec2 mouseDelta = io.MouseDelta;
						ImVec2 delta = mouseDelta / pScale; // Convert back to curve space

						unSnappedCpPos += delta;
						ImVec2 newCpPos = unSnappedCpPos;

						if (snapping)
						{
							// Set control point pos to nearest grid intersection
							newCpPos.x = roundf(newCpPos.x / pointBounds.GetWidth() * gridLines.x) * (pointBounds.GetWidth() / gridLines.x);
							newCpPos.y = roundf(newCpPos.y / pointBounds.GetHeight() * gridLines.y) * (pointBounds.GetHeight() / gridLines.y);
						}

						MoveCP(*cp, *cpMirror, (*points)[i].position, newCpPos, jointed);
						changed = true;

						if (!noLabels)
						{
							// While dragging, draw vector coords to bottom right
							char label[64];
							snprintf(label, 64, "X%.3f\nY%.3f", newCpPos.x, newCpPos.y);
							ImVec2 textSize = CalcTextSize(label);
							drawList->AddText(ImVec2(contentArea.Max.x - textSize.x - 8, contentArea.Max.y - textSize.y - 8), IM_COL32(255, 255, 255, 255), label);
						}
					}

					mouseAbsorbed = true;
				}
			}
		}

		if (!noPoints)
			drawList->AddCircleFilled(pPos, thickness * 1.6f, IM_COL32(80, 255, 80, 255));

		if (!noLabels)
		{
			// Draw point number label
			ImVec2 labelSize = CalcTextSize(std::to_string(i).c_str());
			drawList->AddText(pPos + ImVec2(thickness * 3.0f, -thickness * 3.0f) - labelSize * 0.65f, IM_COL32(255, 255, 255, 255), std::to_string(i + 1).c_str());
		}
	}

	if (!readOnly && !mouseAbsorbed)
	{
		if (IsMouseClicked(ImGuiMouseButton_Left))
		{
			// Check if user has clicked on a curve and add a new point if so
			ImVec2 mousePos = io.MousePos;

			for (int i = 0; i < pointCount - 1; i++)
			{
				int newPointAdded = -1;

				if (linear)
				{
					ImVec2 p1 = (*points)[i].position * pScale + pOffset;
					ImVec2 p2 = (*points)[i + 1ll].position * pScale + pOffset;

					if (!IsMouseHoveringLine(p1, p2, thickness))
						continue;

					ImVec2 closestPoint = ImVec2(
						std::clamp(mousePos.x, std::min(p1.x, p2.x), std::max(p1.x, p2.x)),
						std::clamp(mousePos.y, std::min(p1.y, p2.y), std::max(p1.y, p2.y))
					);

					float distanceSq = ImLengthSqr(mousePos - closestPoint);

					if (distanceSq > thickness * thickness * 2.5f + 1.0f)
						continue;

					// Insert new point into curve
					BezierPoint newPoint;
					newPoint.position = (closestPoint - pOffset) / pScale; // Convert back to curve space
					newPoint.controlPoint1 = newPoint.position - ImVec2(0.1f, 0);
					newPoint.controlPoint2 = newPoint.position + ImVec2(0.1f, 0);

					newPointAdded = i + 1;
					points->insert(points->begin() + newPointAdded, newPoint);
				}
				else if (quadratic)
				{
					// TODO
				}
				else
				{
					ImVec2 p1 = (*points)[i].position * pScale + pOffset;
					ImVec2 p2 = (*points)[i].controlPoint2 * pScale + pOffset;
					ImVec2 p3 = (*points)[i + 1ll].controlPoint1 * pScale + pOffset;
					ImVec2 p4 = (*points)[i + 1ll].position * pScale + pOffset;

					ImVec2 closestPoint = ImBezierCubicClosestPointCasteljau(p1, p2, p3, p4, mousePos, powf(GetStyle().CurveTessellationMaxError, 2.0f));
					float distanceSq = ImLengthSqr(mousePos - closestPoint);

					if (distanceSq > thickness * thickness * 2.5f + 1.0f)
						continue;

					// Find t parameter for closest point
					float t;
					{
						// Sample 20 points along the curve and find the closest one, 
						// then sample 20 more points between the closest point and its neighbors to refine t

						float closestT = 0.0f;
						float closestDistanceSq = FLT_MAX;
						for (int j = 0; j <= 20; j++)
						{
							float sampleT = j / 20.0f;
							ImVec2 samplePoint = ImBezierCubicCalc(p1, p2, p3, p4, sampleT);
							float sampleDistanceSq = ImLengthSqr(mousePos - samplePoint);

							if (sampleDistanceSq < closestDistanceSq)
							{
								closestDistanceSq = sampleDistanceSq;
								closestT = sampleT;
							}
						}

						t = closestT;
						for (int j = 0; j < 20; j++)
						{
							float sampleT1 = t - 0.01f;
							float sampleT2 = t + 0.01f;

							ImVec2 samplePoint1 = ImBezierCubicCalc(p1, p2, p3, p4, sampleT1);
							ImVec2 samplePoint2 = ImBezierCubicCalc(p1, p2, p3, p4, sampleT2);

							float sampleDistanceSq1 = ImLengthSqr(mousePos - samplePoint1);
							float sampleDistanceSq2 = ImLengthSqr(mousePos - samplePoint2);

							if (sampleDistanceSq1 < closestDistanceSq)
							{
								closestDistanceSq = sampleDistanceSq1;
								t = sampleT1;
							}
							else if (sampleDistanceSq2 < closestDistanceSq)
							{
								closestDistanceSq = sampleDistanceSq2;
								t = sampleT2;
							}
							else
							{
								break;
							}
						}
					}

					float boundsDim = std::max(pointBounds.GetWidth(), pointBounds.GetHeight());

					// Insert new point into curve
					BezierPoint newPoint;
					newPoint.position = ImBezierCubicCalc((*points)[i].position, (*points)[i].controlPoint2, (*points)[i + 1ll].controlPoint1, (*points)[i + 1ll].position, t);
					newPoint.controlPoint1 = ImBezierCubicCalc((*points)[i].position, (*points)[i].controlPoint2, (*points)[i + 1ll].controlPoint1, (*points)[i + 1ll].position, t - 0.01f);
					newPoint.controlPoint2 = ImBezierCubicCalc((*points)[i].position, (*points)[i].controlPoint2, (*points)[i + 1ll].controlPoint1, (*points)[i + 1ll].position, t + 0.01f);

					glm::vec2 
						point{newPoint.position.x, newPoint.position.y}, 
						cp1ToCp2{newPoint.controlPoint2.x - newPoint.controlPoint1.x, newPoint.controlPoint2.y - newPoint.controlPoint1.y};

					glm::vec2 tangent = glm::normalize(cp1ToCp2);

					glm::vec2 
						cp1 = tangent * (-0.25f * boundsDim) + point, 
						cp2 = tangent * (0.25f * boundsDim) + point;

					newPoint.controlPoint1 = ImVec2(cp1.x, cp1.y);
					newPoint.controlPoint2 = ImVec2(cp2.x, cp2.y);

					newPointAdded = i + 1;
					points->insert(points->begin() + newPointAdded, newPoint);
				}

				if (newPointAdded != -1)
				{
					// Clear selections
					storage->Clear();

					ImGuiID pointID = window->GetID(std::format("##Point{}", newPointAdded).c_str());
					ImGuiID pointPrevID = window->GetID(std::format("##PointPrev{}", newPointAdded).c_str());
					ImGuiID pointDragID = window->GetID(std::format("##PointDrag{}", newPointAdded).c_str());

					// Select the new point
					storage->SetInt(pointID, 2);
					storage->SetInt(pointPrevID, 0);
					storage->SetInt(pointDragID, 0);

					deltaBuffer = ImVec2(0, 0);
					unSnappedPos = (*points)[newPointAdded];
					mouseAbsorbed = true;
					break;
				}
			}
		}
	}

	// Clear selections if user clicks outside of any points or control points while not holding multi-select
	if (!mouseAbsorbed && !multiSelect)
	{
		if (IsMouseClicked(ImGuiMouseButton_Left))
		{
			if (IsMouseHoveringRect(contentArea.Min, contentArea.Max, true))
			{
				storage->Clear();
				mouseAbsorbed = true;
			}
		}
	}

	// If mouse was absorbed, prevent ex. dragging window
	if (mouseAbsorbed)
	{
		SetKeyOwner(ImGuiKey_MouseLeft, window->ID);
	}

	drawList->PopClipRect();

	EndChild();

	return changed;
}

#endif
