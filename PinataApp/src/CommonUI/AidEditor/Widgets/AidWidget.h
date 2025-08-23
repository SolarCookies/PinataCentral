#pragma once
#include "../../../Utils/ZLibHelpers.h"

class AidEditor;
class BundleReader;

class AidWidget
{
	public:
	AidWidget() = default;
	virtual ~AidWidget() = default;

	virtual void init(AidEditor& aidEditor, vBYTES& vdat, vBYTES& vgpu, BundleReader& bundle) = 0;

	// Method to render the widget
	virtual void Render() = 0;
};