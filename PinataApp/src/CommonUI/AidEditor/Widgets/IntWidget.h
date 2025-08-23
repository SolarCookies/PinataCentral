#pragma once
#include "AidWidget.h"

#include "imgui.h"
#include <vector>
#include <memory>

class IntWidget : public AidWidget
{
public:
	int Value = 0;
	std::string Label = "Unknown";

	IntWidget(std::string LABEL, int VALUE) {
		this->Label = LABEL;
		this->Value = VALUE;
	}
	virtual ~IntWidget() = default;


	virtual void init(AidEditor& aidEditor, vBYTES& vdat, vBYTES& vgpu, BundleReader& bundle){
	}

	// Method to render the widget
	virtual void Render() {
		ImGui::InputInt(Label.c_str(), &Value);
		ImGui::Separator();

	}
};