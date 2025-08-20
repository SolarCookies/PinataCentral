#pragma once
#include "../../Page.h"

#include "FileBrowser/FileBrowser.h"

class PackageManager : public Page
{
public:
	FileBrowser fileBrowser;
	void render(GUI& gui) override;
};