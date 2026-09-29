#pragma once
#include <EditorWindow.hpp>

namespace Droplet::Editor
{
	/// @brief A class that inherits from EditorWindow.
	/// @details This class is supposed to show components from a selected node 
	/// and expose the data to the developer so that they can be edited.
	class NodeInspectorWindow : public EditorWindow
	{
	public:
		NodeInspectorWindow() = default;
		~NodeInspectorWindow() = default;

	protected:
		void InitImpl() override;
		void CloseImpl() override;
		void RenderImpl() override;
		void RenderToolbar() override;
	};
}
