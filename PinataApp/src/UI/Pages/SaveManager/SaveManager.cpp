#include "SaveManager.h"
#include "../../../GlobalSettings.h"
#include "imgui.h"
#include <filesystem>
#include "../../../Utils/ZLibHelpers.h"
#include "Walnut/Image.h"
#include "../../../SaveTools/Save.h"

struct VPSaveHeader {
	char Magic[4];
	int FirstOffset;
	int SecOffset;
};

void SaveManager::render(GUI& gui)
{
	LoadSettings();
	ImGui::Begin("Save Manager");
	if (GardenDirectory != "") {
		//For each file that ends with .Viva display it in a list with the name of the file
		for(const auto& entry : std::filesystem::directory_iterator(GardenDirectory)) {
			if (entry.path().extension() == ".Viva") {
				std::string fileName = entry.path().filename().string();
				
				std::ifstream file(entry.path(), std::ios::binary | std::ios::ate);
				if (!file) {
					std::cout << "Failed to open file: " << entry.path() << std::endl;
					return;
				}
				std::streamsize size = file.tellg();
				file.seekg(0, std::ios::beg);

				std::vector<uint8_t> buffer(size);
				if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
					std::cout << "Failed to read file: " << entry.path() << std::endl;
					return;
				}
				vBYTES SaveBYTES = vBYTES(buffer.begin(), buffer.end());

				
				int offset = 4136;
				std::string Stats = "";
				while (SaveBYTES[offset] != 0) {
					if (SaveBYTES[offset] == 10) {
						Stats += " ";
						offset += 2;
						continue;
					}
					Stats += SaveBYTES[offset];
					offset += 2;
				}

				int firstoffset = 0;
				memcpy(&firstoffset, &SaveBYTES[8], 4);
				VPSaveHeader vps;
				memcpy(&vps, &SaveBYTES[firstoffset], sizeof(VPSaveHeader));
				int NextOffset = vps.FirstOffset;
				if (vps.FirstOffset != vps.SecOffset) { //The save has a PNG thumbnail
					NextOffset = vps.SecOffset;
					vBYTES Image = vBYTES(&SaveBYTES[vps.FirstOffset], &SaveBYTES[vps.SecOffset]);
					//save image
					std::string GardenPNG = GardenDirectory + "/" + fileName + ".PNG";
					if (std::filesystem::exists(GardenPNG)) {
						//Walnut::Image thumb = Walnut::Image(GardenPNG);
						//ImGui::Image(thumb.GetDescriptorSet(), { 160,90 });
					}
					else {
						std::ofstream file(GardenPNG, std::ios::binary | std::ios::ate);
						if (!file) {
							throw std::runtime_error("Failed to open file for writing");
						}
						file.write((char*)Image.data(), Image.size());
						//Walnut::Image thumb = Walnut::Image(GardenPNG);
						//ImGui::Image(thumb.GetDescriptorSet(), { 160,90 });
						file.close();
					}
					//Walnut::Image("Assets/Save.PNG")
					//Render Thumbnail
					ImGui::SameLine();
				}
				else {
					Walnut::Image m_Unknown_Thumbnail = Walnut::Image("Assets/UI_Icon_NoTag.PNG");
					ImGui::Image(m_Unknown_Thumbnail.GetDescriptorSet(), { 160,90 });
					ImGui::SameLine();
				}

				//Read save here
				if (ImGui::Button(fileName.c_str())) {
					std::wstring pathofSave = std::filesystem::path(entry.path()).wstring();
					ReadSaveFile(pathofSave);
				}

				ImGui::SameLine();
				ImGui::Text(Stats.c_str());

				file.close();

			}
		}
	}
	else {
		ImGui::Text("Garden directory not set.");
	}
	ImGui::End();
}
