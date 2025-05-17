#pragma once

#include "imgui.h"
#include "../../Utils/ZlibHelpers.h"
#include "../../Utils/OpenFileDialog.h"
#include "../../Utils/Log.hpp"
#include "../../PKG/pkg.h"

//Parent to all windows that have settings to adjust .vdat properties
class DataSettingsWindow
{
public:
	DataSettingsWindow(BYTES Data, PKG pkg, uint32_t CAFFNumber, std::string ChunkName) {
		OGData = Data;
		Chunkpkg = pkg;
		ChunkCAFFNumber = CAFFNumber;
		ChunkNameSTR = ChunkName;
	};
	~DataSettingsWindow() {};

	//This is the function that should be overridden to render the settings for the window based on the type of .vdat
	virtual void RenderChild();

	//Render the window from Main (This shouldn't be overridden and contains the logic for all VDAT windows)
	void RenderParent()
	{
		//if window is open
		const char* title = ChunkNameSTR.c_str();
		if (ImGui::Begin(title))
		{
			//Begin Title menu bar
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("Save"))
				{
					ApplyChanges();
				}
				if (ImGui::BeginMenu("Revert Changes"))
				{
					ModData = OGData;
				}
				ImGui::EndMenuBar();
			}
			
			//if the window is open, render the settings
			RenderChild();
		}
		ImGui::End();
	}
	
	virtual BYTES GetModifiedData() { return ModData; }

	virtual void ApplyChanges();
		

private:
	BYTES OGData;
	BYTES ModData;
	PKG Chunkpkg;
	uint32_t ChunkCAFFNumber;
	std::string ChunkNameSTR;

};