#pragma once
#include <SceneSystem/Component.hpp>

namespace Droplet::Scene
{
	class UIComponent : public Component
	{
	public:
		//DOTO more relevent functions 
	protected:
		void UpdateImpl();

		void RenderUIImpl();
		nlohmann::json SerializeImpl();

		void DeserializeImpl();

	private:
	};
}