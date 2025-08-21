#include "BundleManager.h"
#include "../../../GlobalSettings.h"
#include "../../../Utils/Log.hpp"



void BundleManager::render(GUI& gui)
{
	fileBrowser.RenderFileBrowser();

	ImGui::Begin("Log");
	if (ImGui::Button("Clear Log"))
	{
		ClearLog();
	}
	ImGui::SameLine();
	if (ImGui::Button("Copy Log"))
	{
		//copy log to clipboard
		std::string log = "";
		for (int i = 0; i < Logs.size(); i++)
		{
			if (Keys[i] == " ")
				log += Logs[i] + "\n";
			else
				log += Logs[i] + " Type: " + Keys[i] + "\n";
		}
		ImGui::SetClipboardText(log.c_str());
	}
	DrawLog();
	ImGui::End();

}
