
#include "CameraComponent.hpp"

#include <SceneSystem/Node.hpp>

#include <glm/gtc/constants.hpp>
#include <glm/common.hpp>
#include <ImGui/imgui.h>
#include <Time.hpp>
#include <cmath>

namespace
{
	float WrapDegrees(float a)
	{
		while (a > 180.0f)  a -= 360.0f;
		while (a < -180.0f) a += 360.0f;
		return a;
	}

	
	// Rate: higher = snappier. At or above kInstantRate there is no smoothing.
	constexpr float kInstantRate = 500.0f;

	float Smooth(float rate, float dt)
	{
		if (rate >= kInstantRate)
			return 1.0f;                        // no delay
		return 1.0f - std::exp(-rate * dt);
	}
}

using namespace Droplet::Scene;
using namespace Droplet::Graphics;

void CameraComponent::StartImpl()
{
	// TODO: Construct camera and create render target
}

void CameraComponent::LateUpdateImpl()
{
	Transform &t = GetOwner()->GetTransform();
}

void CameraComponent::QueueRenderImpl()
{
	// TODO
}

void CameraComponent::RenderUIImpl()
{
	// TODO
	// Aspect
	// fov, 
	// near/far planes
	// Render target view

	DrawLayerUtility::SetLayerMaskUI("Draw Layers", m_drawLayers);
}

nlohmann::json CameraComponent::SerializeImpl()
{
	// TODO
	return nlohmann::json::object();
}

void CameraComponent::DeserializeImpl(nlohmann::json p_compJson)
{
	// TODO
}
