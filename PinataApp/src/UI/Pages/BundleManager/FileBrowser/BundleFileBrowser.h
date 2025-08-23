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
#include "../../../../CommonUI/AidEditor/AidEditor.h"
#include "../../../../CommonUI/AidEditor/Widgets/LocalizationWidget.h"
#include <random>

class BundleFileBrowser
{
public:

	BundleReader bundle;
	vBYTES CurrentBNLBytes;
	std::string CurrentBNLName;
	AidEditor aidEditor;
	
	
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
            // Search bar
            ImGui::InputText("Search", Searchbuf, sizeof(Searchbuf));
            SearchTerm = std::string(Searchbuf);

            for (int i = 0; i < bundle.bvref.ChunkNames.size(); i++)
            {
                //get everything before the first "," in the string
                std::string chunkNameFull = bundle.bvref.ChunkInfos[i].ChunkName;
                size_t commaPos = chunkNameFull.find(',');
                std::string chunkName = (commaPos != std::string::npos) ? chunkNameFull.substr(0, commaPos) : chunkNameFull;

                // Filter by search term (case-insensitive)
                if (!SearchTerm.empty()) {
                    std::string chunkNameLower = chunkName;
                    std::string searchTermLower = SearchTerm;
                    std::transform(chunkNameLower.begin(), chunkNameLower.end(), chunkNameLower.begin(), ::tolower);
                    std::transform(searchTermLower.begin(), searchTermLower.end(), searchTermLower.begin(), ::tolower);
                    if (chunkNameLower.find(searchTermLower) == std::string::npos)
                        continue;
                }

                if (ImGui::Button(std::string("View " + chunkName).c_str())) {
                    CurrentBNLName = chunkName;
                    CurrentBNLBytes.resize(bundle.bvref.ChunkInfos[i].VDAT_Size);
                    vBYTES vdat = caff::Get_VDAT(bundle.CAFFBYTES, bundle.bcaff, bundle.bvref);
                    vBYTES chunkData;
                    chunkData.resize(bundle.bvref.ChunkInfos[i].VDAT_Size);
                    std::memcpy(chunkData.data(), vdat.data() + bundle.bvref.ChunkInfos[i].VDAT_Offset, bundle.bvref.ChunkInfos[i].VDAT_Size);
                    CurrentBNLBytes = chunkData;
                    aidEditor.aidWidgets.clear();

                    auto localizationWidget = std::make_unique<LocalizationWidget>();
                    localizationWidget->setLocalizationData(CurrentBNLBytes);
                    aidEditor.AddWidget(localizationWidget.release());
                    aidEditor.CurrentFileName = CurrentBNLName;
                }
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

        aidEditor.Render();

    }
};