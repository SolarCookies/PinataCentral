#pragma once
#include "AidWidget.h"
#include "aid_loctext.h"

#include "imgui.h"
#include <vector>
#include <memory>

class LocalizationWidget : public AidWidget
{
public:
	LocalizationWidget() = default;
	virtual ~LocalizationWidget() = default;

	Aid_Loctext Localization;

	// Method to render the widget
	virtual void Render() {
		ImGui::BeginChild("Localization Widget");
		for (const auto& entry : Localization.Tags) {
			std::string displayText = entry + ": ";
			ImGui::TextWrapped("%s", displayText.c_str());
			ImGui::SameLine();
			std::string locText = Localization.GetTextByTag(entry);
			std::vector<char> buffer(locText.begin(), locText.end());
			buffer.push_back('\0');
			
			ImGui::InputText(("##" + entry).c_str(), buffer.data(), buffer.size());
			if (ImGui::IsItemHovered()) {
				ImGui::SetNextWindowSizeConstraints(ImVec2(300, 0), ImVec2(600, FLT_MAX)); // Set min/max width
				ImGui::BeginTooltip();
				ImGui::TextWrapped("%s", locText.c_str());
				ImGui::EndTooltip();
			}

			ImGui::Separator();
		}
		ImGui::EndChild();

	}

	void setLocalizationData(const vBYTES& data) {
		Localization = Aid_Loctext(data);
	}
};