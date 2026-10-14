#include "EditorWindow.hpp"
#include <tracy/public/tracy/Tracy.hpp>

using namespace Droplet::Editor;


void EditorWindow::Init()
{
	ZoneScoped;

	InitImpl();

	ZoneText(m_name.c_str(), m_name.size());
}

void EditorWindow::Close()
{
	ZoneScoped;
	ZoneText(m_name.c_str(), m_name.size());

	CloseImpl();
}

void EditorWindow::Render()
{
	ZoneScoped;
	ZoneText(m_name.c_str(), m_name.size());

	OpenWindow();

	if (m_hasToolbar)
	{
		if (ImGui::BeginMenuBar())
		{
			ZoneNamedN(windowToolbarZone, "Toolbar", true);

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
