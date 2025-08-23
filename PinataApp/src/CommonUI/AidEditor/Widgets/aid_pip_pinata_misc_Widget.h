#pragma once
#include "AidWidget.h"
#include "../../../Bundle.h"
#include "LocalizationWidget.h"
#include "../AidEditor.h"

#include "imgui.h"
#include <vector>
#include <memory>

class PipWidget : public AidWidget
{
public:
	PipWidget() = default;
	virtual ~PipWidget() = default;

	vBYTES Localvdat;

	virtual void init(AidEditor& aidEditor, vBYTES& vdat, vBYTES& vgpu, BundleReader& bundle);

	// Method to render the widget
	virtual void Render() {

	}

};