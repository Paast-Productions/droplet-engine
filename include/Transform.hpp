#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>

namespace Droplet::Scene
{
	class Node; // Forward declaration of Node class

	/// @brief A class that represents a 3D transformation, including position, rotation, and scale.
	class Transform
	{
	public:
		enum class Space
		{
			Local,
			World
		};

		/// @brief Construct a new Transform object.
		/// @param p_owner A pointer to the Node that owns this Transform.
		Transform(Node *p_owner);

		~Transform() = default;

		// ================================ Getters ================================

		/// @brief Get the position of the transform in the specified space.
		/// @param p_space The space in which to get the position. Defaults to local space.
		/// @return The position of the transform in the specified space.
		[[nodiscard]] glm::vec3 GetPosition(Space p_space = Space::Local) const;

		/// @brief Get the rotation of the transform in the specified space.
		/// @param p_space The space in which to get the rotation. Defaults to local space.
		/// @return The rotation of the transform in the specified space.
		[[nodiscard]] glm::quat GetRotation(Space p_space = Space::Local) const;

		/// @brief Get the Euler angles of the transform in the specified space.
		/// @param p_space The space in which to get the Euler angles. Defaults to local space.
		/// @return The Euler angles of the transform in the specified space.
		[[nodiscard]] glm::vec3 GetEuler(Space p_pspace = Space::Local) const;

		/// @brief Get the local scale of the transform.
		/// @return The local scale of the transform.
		[[nodiscard]] glm::vec3 GetScale() const;

		/// @brief Get the transformation matrix of the transform in the specified space.
		/// @param p_space The space in which to get the matrix. Defaults to local space.
		/// @return The transformation matrix of the transform in the specified space.
		[[nodiscard]] glm::mat4 GetMatrix(Space p_space = Space::Local) const;

		/// @brief Check if the transform is dirty (i.e., if it has been modified since the last update).
		/// @return True if the transform is dirty, false otherwise.
		[[nodiscard]] bool IsDirty() const;

		/// @brief Get the up direction of the transform in the specified space.
		/// @param p_space The space in which to get the up direction. Defaults to local space.
		/// @return The up direction of the transform in the specified space.
		[[nodiscard]] glm::vec3 GetUp(Space p_space = Space::Local) const;

		/// @brief Get the right direction of the transform in the specified space.
		/// @param p_space The space in which to get the right direction. Defaults to local space.
		/// @return The right direction of the transform in the specified space.
		[[nodiscard]] glm::vec3 GetRight(Space p_space = Space::Local) const;

		/// @brief Get the forward direction of the transform in the specified space.
		/// @param p_space The space in which to get the forward direction. Defaults to local space.
		///	@return The forward direction of the transform in the specified space.
		[[nodiscard]] glm::vec3 GetForward(Space p_space = Space::Local) const;

		// ================================ Setters ================================

		/// @brief Set the position of the transform in the specified space.
		/// @param p_position The new position to set.
		/// @param p_space The space in which to set the position. Defaults to local space.
		void SetPosition(const glm::vec3 &p_position, Space p_space = Space::Local);

		/// @brief Set the rotation of the transform in the specified space.
		/// @param p_rotation The new rotation to set.
		/// @param p_space The space in which to set the rotation. Defaults to local space.
		void SetRotation(const glm::quat &p_rotation, Space p_space = Space::Local);

		/// @brief Set the Euler angles of the transform in the specified space.
		/// @param p_eulerAngles The new Euler angles to set.
		/// @param p_space The space in which to set the Euler angles. Defaults to local space.
		void SetEuler(const glm::vec3 &p_eulerAngles, Space p_space = Space::Local);

		/// @brief Set the local scale of the transform.
		/// @param p_scale The new scale to set.
		void SetScale(const glm::vec3 &p_scale);

		/// @brief Set the transformation matrix of the transform in the specified space.
		/// @param p_matrix The transformation matrix to set.
		/// @param p_space The space in which to set the matrix. Defaults to local space.
		void SetMatrix(const glm::mat4 &p_matrix, Space p_space = Space::Local);

		/// @brief Mark the transform as dirty, indicating that it has been modified and needs to be updated.
		void MakeDirty();

		// ================================ Utility ================================

		/// @brief Update the world & local matrices of the transform based on its position, rotation, scale and the parent's world matrix.
		/// Sets the transform as clean after updating. Will update recursively if the parent transform is dirty.
		/// It is up to the transform owner to ensure this is called at the right time.
		void RecalculateMatrices();

		/// @brief Move the transform by a specified delta in the specified space.
		/// @param p_delta The delta by which to move the transform.
		/// @param p_space The space in which to perform the move operation. Defaults to local space.
		void Move(const glm::vec3 &p_delta, Space p_space = Space::Local);

		/// @brief Rotate the transform by a specified delta in the specified space.
		/// @param p_delta The delta by which to rotate the transform.
		/// @param p_space The space in which to perform the rotate operation. Defaults to local space.
		void Rotate(const glm::quat &p_delta, Space p_space = Space::Local);

		/// @brief Rotate the transform by a specified delta in Euler angles in the specified space.
		/// @param p_delta The delta by which to rotate the transform in Euler angles.
		/// @param p_space The space in which to perform the rotate operation. Defaults to local space.
		void RotateEuler(const glm::vec3 &p_delta, Space p_space = Space::Local);

		/// @brief Add a scale to the transform in local space.
		/// @param p_delta The delta to add to the current scale of the transform.
		void AddScale(const glm::vec3 &p_delta);

		/// @brief Rotate the transform around a specified axis by a specified angle in the specified space.
		/// @param p_angle The angle in radians by which to rotate the transform.
		/// @param p_axis The axis around which to rotate the transform.
		/// @param p_space The space in which to perform the rotate operation. Defaults to local space.
		void RotateAxis(float p_angle, const glm::vec3 &p_axis, Space p_space = Space::Local);

		/// @brief Make the transform look at a target position.
		/// @param p_target The target position to look at.
		/// @param p_up The up direction for the transform. Defaults to (0, 1, 0).
		/// @param p_space The space in which to perform the look-at operation. Defaults to local space.
		void LookAt(const glm::vec3 &p_target, const glm::vec3 &p_up = glm::vec3(0.f, 1.f, 0.f), Space p_space = Space::Local);

	private:
		Node					*m_owner{ nullptr }; // Raw pointer because it will by definition always be valid.
		bool					m_isDirty{ true };

		glm::vec3				m_position{ 0.f, 0.f, 0.f };
		glm::quat				m_rotation{ 1.f, 0.f, 0.f, 0.f };
		glm::vec3				m_scale{ 1.f, 1.f, 1.f };

		glm::mat4				m_localMatrix{ 1.f };
		glm::mat4				m_worldMatrix{ 1.f };

		/// @brief Validate the rotation of the transform to ensure it is a valid quaternion.
		void ValidateRotation();

		/// @brief Update the local matrix of the transform based on its position, rotation, and scale.
		void UpdateLocalMatrix();

		[[nodiscard]] bool HasParent() const;
		[[nodiscard]] Transform *GetParentTransform() const;
	};
}