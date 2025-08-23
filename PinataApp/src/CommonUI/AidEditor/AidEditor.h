#pragma once
#include "Widgets/AidWidget.h"
#include "imgui.h"
#include <vector>
#include <memory>

class AidEditor
{
public:

	std::vector<std::unique_ptr<AidWidget>> aidWidgets;
	std::string CurrentFileName = "";

	AidEditor() = default;
	~AidEditor() = default;
	void AddWidget(AidWidget* widget)
	{
		aidWidgets.emplace_back(widget);
	}
	void Render() {
		ImGui::Begin("Aid Editor");
		ImGui::Text("Current File: %s", CurrentFileName.c_str());
		ImGui::Separator();
		ImGui::BeginChild("AidWidgets", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
		for (auto& widget : aidWidgets) {
			if (widget) {
				widget->Render();
			}
		}
		ImGui::EndChild();
		ImGui::End();
	}
};
