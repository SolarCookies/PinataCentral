#pragma once
#include "../../Utils/OpenFIleDialog.h"
#include "../Page.h"
#include "../../PKG/pkg.h"

class ExtractPage : public Page {
public:
	std::string Path = "";
	int PKGIndex = -1;
	char filter[256];
	void render(GUI& gui) override;
};
