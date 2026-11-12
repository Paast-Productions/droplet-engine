#pragma once
#include <SceneSystem/Component.hpp>
#include <string_view>
#include <memory>

namespace Droplet::Scene
{
	class PodBehaviour;

	class PodComponent : public Component
	{
	public:
		[[nodiscard]] std::string_view GetTypeName() const override { return "PodComponent"; }

		~PodComponent();

	protected:
		void StartImpl() override;

		void RenderUIImpl() override;

	private:
		std::weak_ptr<PodBehaviour> m_podBehaviour;
	};
}