#pragma once
#include "imgui.h"

static inline bool bTimeSettings = false;

class CommonPatches
{
	public:

	static void Render()
	{
		return;
		/*
		ImGui::Begin("Common Patches", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
		if(ImGui::Button("Time Settings"))
		{
			bTimeSettings = !bTimeSettings;
		}
		ImGui::End();

		if(bTimeSettings)
		{
			ImGui::Begin("Time Settings", &bTimeSettings, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
			ImGui::Text("This is where you can adjust time settings.");
			// Add more time settings controls here
			ImGui::End();
		}
		*/
	}

};