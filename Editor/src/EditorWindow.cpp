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
		if (ImGui::BeginMenuBar())
		{
			RenderToolbar();
			ImGui::EndMenuBar();
		}
	}

	RenderImpl();

	CloseWindow();
}

void EditorWindow::ToggleToolbar(bool p_visible)
{
	m_hasToolbar = p_visible;

	if (p_visible)
	{
		m_windowFlags |= ImGuiWindowFlags_MenuBar;
	}
	else
	{
		m_windowFlags &= ~ImGuiWindowFlags_MenuBar;
	}
}

void EditorWindow::OpenWindow()
{
	ImGui::Begin(m_name.c_str(), nullptr, m_windowFlags);
}

void EditorWindow::CloseWindow()
{
	ImGui::End();
}
