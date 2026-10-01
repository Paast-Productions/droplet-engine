#include "TransformUI.hpp"
#include <ImGui/imgui.h>
#include <string>
#include <stdexcept>

using namespace Droplet::Editor;
using namespace Droplet::Scene;

constexpr float RAD_TO_DEG = 180.0f / 3.14159265358979323846f;
constexpr float DEG_TO_RAD = 3.14159265358979323846f / 180.0f;

bool Droplet::Editor::ShowTransformUI(Transform *p_transform)
{
	using namespace ImGui;

	if (!p_transform)
	{
		throw std::invalid_argument("p_transform cannot be null");
	}

	PushID("TransformUI");

	bool edited = false;

	if (CollapsingHeader("Transform"))
	{
		// TODO: Add settings such as reset buttons

		Transform::Space space = Transform::Space::Local; // HACK: Get this value from a shared state.

		float width = GetContentRegionAvail().x;

		// Display main transform properties (pos, euler, scale)
		glm::vec3 position = p_transform->GetPosition(space);
		glm::vec3 euler = p_transform->GetEuler(space) * RAD_TO_DEG;
		glm::vec3 scale = p_transform->GetScale();
		
		ImGui::SetNextItemWidth(width);

		if (DragFloat3("##Position", &position.x, 0.02f))
		{
			p_transform->SetPosition(position, space);
			edited = true;
		}

		ImGui::SetNextItemWidth(width);

		if (DragFloat3("##Rotation", &euler.x, 0.1f))
		{
			p_transform->SetEuler(euler * DEG_TO_RAD, space);
			edited = true;
		}

		ImGui::SetNextItemWidth(width);

		if (DragFloat3("##Scale", &scale.x, 0.01f))
		{
			p_transform->SetScale(scale);
			edited = true;
		}
	}

	PopID();

	return edited;
}