#pragma once
#include "../../Page.h"

#include "FileBrowser/BundleFileBrowser.h"

class BundleManager : public Page
{
public:
	BundleFileBrowser fileBrowser;
	void render(GUI& gui) override;
};