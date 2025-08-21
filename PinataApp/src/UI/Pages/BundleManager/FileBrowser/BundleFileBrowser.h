#pragma once

#include "imgui.h"
#include "Walnut/Application.h"

#include "../../../../PKG/pkg.h"
#include "../../PackageManager/FileBrowser/imgui_memory_editor.h"
#include <vulkan/vulkan.h>
#include "../../../../Utils/dds.hpp"
#include <filesystem>
#include "../../../../PKG/OGModel.h"
#include "../../../../GlobalSettings.h"
#include "../../../../Bundle.h"

#include <random>

class BundleFileBrowser
{
public:

	BundleReader bundle;
	
	
	BundleFileBrowser() {
		LoadSettings();
		std::string BundlesDicrectoryEnglish = BundlesDicrectory + "\\englishus.bnl";
		bundle = BundleReader(BundlesDicrectoryEnglish);
	};
	~BundleFileBrowser() = default;
	
	
	std::string CurrentCAFF = "";
	std::string CurrentAid = "";

	bool ShowImageWindow = false;

	bool UpdateImage = false;
	bool ShowRaw = false;

	VkDescriptorSet* desc;

	char Searchbuf[255]{};
	std::string SearchTerm = "";

	Walnut::Image m_Tag_Thumbnail = Walnut::Image("Assets/UI_Icon_TagGroup.PNG");

	bool ShowHexWindow = false;
	bool FoundFiles = false;
	vBYTES HexData;
	std::vector<std::string> files;
	std::vector<std::string> FileNames;

	void RenderFileBrowser() {

		//File Browser Window
		if (ImGui::Begin("Bundle Browser"))
		{
			for(int i = 0; i < bundle.bvref.ChunkNames.size(); i++)
			{
				ImGui::Text("%s", bundle.bvref.ChunkNames[i].c_str());
			}
		}
		ImGui::End();

		if (ShowHexWindow)
		{
			static MemoryEditor hex_edit;
			if (!hex_edit.DrawWindow("Hex Editor", HexData.data(), HexData.size(), CurrentAid.c_str()))
			{
				ShowHexWindow = false;
			}
		}

	}

};