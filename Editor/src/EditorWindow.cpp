#include "EditorWindow.hpp"

using namespace Droplet::Editor;


void EditorWindow::Init()
{

	InitImpl();
}

void EditorWindow::Close()
{

	CloseImpl();
}

void EditorWindow::Render()
{
	OpenWindow();

	if (m_hasToolbar)
	{
		RenderToolbar();
	}

	RenderImpl();

	CloseWindow();
}

void EditorWindow::OpenWindow()
{
	ImGui::Begin(m_name.c_str(), nullptr, m_windowFlags);
}

void EditorWindow::CloseWindow()
{
	ImGui::End();
}
