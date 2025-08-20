#pragma once
#include "../../Utils/OpenFIleDialog.h"
#include "../Page.h"

class EditorPage : public Page {
public:
	std::string Path = "";
	void render(GUI& gui) override;
};