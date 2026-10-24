// Any files that are not frequently changed and are used across multiple source files can be included in a precompiled header (PCH) to speed up compilation times. 
// This file serves as a precompiled header for the editor module of the project, including commonly used headers and external libraries.

#pragma once

#include <PCH/GraphicsPCH.h>

// Externals

// Editor
#include <core/FileExplorer.hpp>
#include <ImGui/ImGuiUtils.hpp>
#include <ImGui/UIStyle.hpp>
#include <Scene/ComponentList.hpp>
#include <EditorConfig.hpp>
#include <EditorWindow.hpp>
#include <InteractionState.hpp>
