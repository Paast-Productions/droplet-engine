#pragma once

#include "EditorWindow.hpp"

namespace Droplet::Editor
{
	class BusInspectorWindow : public EditorWindow
	{
	public:
		BusInspectorWindow() = default;
		~BusInspectorWindow() = default;
	protected:

		void InitImpl() override;
		void CloseImpl() override;
		void RenderImpl() override;

	private:

	};
}