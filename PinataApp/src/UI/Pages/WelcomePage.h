#pragma once
#include "../../Utils/OpenFIleDialog.h"
#include "../Page.h"

class WelcomePage : public Page {
	public:
		std::string Path = "";
		void render(GUI &gui) override;
};
