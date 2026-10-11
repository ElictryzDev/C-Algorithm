#pragma once

#include "../Core/ProjectManager.hpp"
#include "../vendor/imgui/imgui.h"

void RenderProjectGatingModal(ProjectManager& pm);
void RenderFlowGraphWorkspace(ProjectManager& pm, int& selectedNodeId);
void RenderAdvisorWorkspace(ProjectManager& pm);
void RenderCodegenWorkspace(ProjectManager& pm);
void RenderCustomPrintScene(ProjectManager& pm);
void RenderSettingsWorkspace(ProjectManager& pm);
void RenderInspectorPanel(ProjectManager& pm, int& selectedNodeId);
void ApplyAppTheme(AppTheme theme);
