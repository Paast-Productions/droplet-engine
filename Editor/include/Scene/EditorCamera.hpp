#pragma once

namespace Droplet::Editor::Scene
{
	/// @brief Represents a camera used in the editor for viewing and navigating the scene.
	class EditorCamera
	{
	public:
		EditorCamera() = default;
		~EditorCamera() = default;

		void Update();

		/// @brief Gets the horizontal field of view (FOV) of the camera in degrees.
		[[nodiscard]] float GetFOV() const { return m_fov; }

		/// @brief Gets the aspect ratio of the camera. The aspect ratio is defined as the width divided by the height of the camera's view.
		[[nodiscard]] float GetAspect() const { return m_aspect; }

	private:
		// TODO: position, rotation, etc.

		float m_fov = 45.0f;		// horizontal field of view in degrees
		float m_aspect = 1.0f;		// width / height
	};

}