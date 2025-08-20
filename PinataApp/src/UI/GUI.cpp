#include "GUI.h"
#include "Pages/WelcomePage.h"
#include "../GlobalSettings.h"
#include "Pages/EditorSelectionPage.h"

void GUI::init()
{
	LoadSettings();
	if(RootDicrectory.empty())
	{
		CurrentPage = std::make_unique<WelcomePage>();
	}
	else {
		CurrentPage = std::make_unique<EditorPage>();
	}
	SaveSettings();
	HasInitialized = true;
}

void GUI::render()
{
	if (CurrentPage) CurrentPage->render(*this);
}
