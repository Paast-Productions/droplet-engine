#pragma once
#include <EditorConfig.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <cmath>

namespace Droplet::Editor
{
	/// @brief Snap increments applied to node transforms.
	/// @details Values are persisted in the editor config and edited from the Scene View's "Edit > Snapping" menu.
	/// Snapping is toggled by `enabled`, and holding Ctrl inverts that state for the duration of the drag.
	struct TransformSnapSettings
	{
		static constexpr const char *EnabledKey = "Snap.Enabled";
		static constexpr const char *AbsoluteKey = "Snap.Absolute";
		static constexpr const char *TranslateKey = "Snap.Translate";
		static constexpr const char *RotateKey = "Snap.Rotate";
		static constexpr const char *ScaleKey = "Snap.Scale";
		static constexpr const char *BoundsKey = "Snap.Bounds";

		/// @brief Whether snapping is on when Ctrl is not held. Holding Ctrl inverts this.
		bool enabled = false;
		/// @brief If true, snap to multiples measured from the world origin. If false, snap relative to the transform at the start of the drag.
		bool absolute = false;
		/// @brief Grid size for position snapping, in world units.
		float translate = 1.0f;
		/// @brief Increment for rotation snapping, in degrees.
		float rotate = 15.0f;
		/// @brief Increment for scale snapping.
		float scale = 0.1f;
		/// @brief Increment for bounds resizing.
		float bounds = 0.1f;

		/// @brief Returns whether snapping applies given the current Ctrl state.
		bool IsActive(bool p_ctrlHeld) const
		{
			return enabled != p_ctrlHeld;
		}

		static TransformSnapSettings Load()
		{
			TransformSnapSettings settings;
			settings.enabled = EditorConfig::LoadBool(EnabledKey, settings.enabled);
			settings.absolute = EditorConfig::LoadBool(AbsoluteKey, settings.absolute);
			settings.translate = EditorConfig::LoadFloat(TranslateKey, settings.translate);
			settings.rotate = EditorConfig::LoadFloat(RotateKey, settings.rotate);
			settings.scale = EditorConfig::LoadFloat(ScaleKey, settings.scale);
			settings.bounds = EditorConfig::LoadFloat(BoundsKey, settings.bounds);
			return settings;
		}

		void Store() const
		{
			EditorConfig::StoreBool(EnabledKey, enabled);
			EditorConfig::StoreBool(AbsoluteKey, absolute);
			EditorConfig::StoreFloat(TranslateKey, translate);
			EditorConfig::StoreFloat(RotateKey, rotate);
			EditorConfig::StoreFloat(ScaleKey, scale);
			EditorConfig::StoreFloat(BoundsKey, bounds);
		}
	};

	/// @brief Rounds a value to the nearest multiple of an increment, measured from an origin.
	/// @details Returns the value unchanged for non-positive increments.
	inline float SnapToIncrement(float p_value, float p_increment, float p_origin = 0.0f)
	{
		return p_increment > 0.0f ? p_origin + std::round((p_value - p_origin) / p_increment) * p_increment : p_value;
	}

	/// @brief The kind of transform a gizmo handle edits, used to choose which increment applies.
	enum class SnapCategory
	{
		None,
		Translate,
		Rotate,
		Scale
	};

	/// @brief Snaps the transform in a world matrix to absolute increments of the given category.
	/// @details Translation snaps to the world grid, rotation snaps Euler angles in degrees, and scale snaps each axis.
	inline void SnapMatrixAbsolute(glm::mat4 &p_matrix, SnapCategory p_category, const TransformSnapSettings &p_settings)
	{
		if (p_category == SnapCategory::Translate)
		{
			for (int i = 0; i < 3; ++i)
			{
				p_matrix[3][i] = SnapToIncrement(p_matrix[3][i], p_settings.translate);
			}
			return;
		}

		if (p_category != SnapCategory::Rotate && p_category != SnapCategory::Scale)
		{
			return;
		}

		glm::vec3 scale, translation, skew;
		glm::quat orientation;
		glm::vec4 perspective;
		if (!glm::decompose(p_matrix, scale, orientation, translation, skew, perspective))
		{
			return;
		}

		if (p_category == SnapCategory::Rotate)
		{
			glm::vec3 euler = glm::degrees(glm::eulerAngles(orientation));
			for (int i = 0; i < 3; ++i)
			{
				euler[i] = SnapToIncrement(euler[i], p_settings.rotate);
			}
			orientation = glm::quat(glm::radians(euler));
		}
		else
		{
			for (int i = 0; i < 3; ++i)
			{
				scale[i] = SnapToIncrement(scale[i], p_settings.scale);
			}
		}

		p_matrix = glm::translate(glm::mat4(1.0f), translation) * glm::mat4_cast(orientation) * glm::scale(glm::mat4(1.0f), scale);
	}
}
