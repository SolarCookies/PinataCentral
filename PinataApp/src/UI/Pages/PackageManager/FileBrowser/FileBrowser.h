#pragma once

#include "imgui.h"
#include "Walnut/Application.h"

#include "../../../../PKG/pkg.h"
#include "imgui_memory_editor.h"
#include <vulkan/vulkan.h>
#include "../../../../Utils/dds.hpp"
#include <filesystem>
#include "../../../../PKG/OGModel.h"
#include "../../../../GlobalSettings.h"
#include "../../../../CommonUI/AidEditor/AidEditor.h"
#include "../../../../CommonUI/AidEditor/Widgets/aid_pip_pinata_misc_Widget.h"
#include "../../../../Bundle.h"

#include <random>

//Enum for render mode
enum class RenderMode
{
	List,
	Grid
};

class FileBrowser
{
public:
	char Bundlepath[255] = { 0 };
	AidEditor aidEditor;
	BundleReader bundle1;

	FileBrowser() {
		LoadSettings();
		for(int i = 0; i < 255; i++) {
			Bundlepath[i] = '\0';
		}
		for(int i = 0; i < PackagesDicrectory.length(); i++) {
			Bundlepath[i] = PackagesDicrectory[i];
		}
		std::string BundlesDicrectoryEnglish = BundlesDicrectory + "\\englishus.bnl";
		bundle1 = BundleReader(BundlesDicrectoryEnglish);
	};
	~FileBrowser() {
	};
	
	void UpdateSelectedAid(std::string Name, vBYTES& vdat, vBYTES& vgpu) {

		aidEditor.aidWidgets.clear();
		if (Name.find("2.53") != std::string::npos) {
			//Requirement
		}
		if (Name.find("2.19") != std::string::npos) {
			//Tag Group
		}
		if (Name.find("2.42") != std::string::npos) {
			//Image
		}
		if (Name.find("4.00") != std::string::npos) {
			//Model
		}
		if (Name.find("4.01") != std::string::npos) {
			//Model TIP
		}
		if (Name.find("2.78") != std::string::npos) {
			auto pipWidget = std::make_unique<PipWidget>();
			pipWidget->init(aidEditor, vdat, vgpu, bundle1);
			aidEditor.AddWidget(pipWidget.release());
		}
	}


	std::string CurrentPKG = "";
	PKG pkg;
	std::string CurrentCAFF = "";
	std::string CurrentAid = "";

	bool ShowImageWindow = false;

	bool UpdateImage = false;
	bool ShowRaw = false;

	VkDescriptorSet* desc;

	char Searchbuf[255]{};
	std::string SearchTerm = "";


	//image thumbnails
	Walnut::Image m_Unknown_Thumbnail = Walnut::Image("Assets/UI_Icon_NoTag.PNG");
	Walnut::Image m_Requirement_Thumbnail = Walnut::Image("Assets/UI_Icon_Requirement.PNG");
	Walnut::Image m_Tag_Thumbnail = Walnut::Image("Assets/UI_Icon_TagGroup.PNG");
	Walnut::Image m_Texture_Thumbnail = Walnut::Image("Assets/UI_Icon_Texture.PNG");
	Walnut::Image m_Model_Thumbnail = Walnut::Image("Assets/UI_Icon_Model.PNG");
	Walnut::Image m_UnknownTexture_Thumbnail = Walnut::Image("Assets/UI_Icon_Texture_Unknown.PNG");

	//Walnut::Image TestDDS;

	bool ShowHexWindow = false;
	bool FoundFiles = false;
	vBYTES HexData;
	std::vector<std::string> files;
	std::vector<std::string> FileNames;

	Walnut::Image* GetAssetThumbnail(std::string ChunkName, bool isDDS)
	{
		if (ChunkName.find("2.53") != std::string::npos) {
			return &m_Requirement_Thumbnail;
		}
		if (ChunkName.find("2.19") != std::string::npos) {
			return &m_Tag_Thumbnail;
		}
		if (ChunkName.find("2.42") != std::string::npos) {
			if (isDDS) {
				return &m_Texture_Thumbnail;
			}
			return &m_UnknownTexture_Thumbnail;
		}
		if (ChunkName.find("4.00") != std::string::npos) {
			return &m_Model_Thumbnail;
		}
		if (ChunkName.find("4.01") != std::string::npos) {
			return &m_Model_Thumbnail;
		}

		return &m_Unknown_Thumbnail;
	}

	void reloadPKG()
	{
		if (CurrentPKG != "")
		{
			//pkg::ReloadPKG(CurrentPKG, pkg);
			CurrentCAFF = "";
			CurrentAid = "";
			CurrentPKG = "";
			FoundFiles = false;
			//pkg = PKG();
			HexData.clear();

		}
	}

	void RenderFileBrowser(RenderMode r) {

		//File Browser Window
		if (ImGui::Begin("File Browser"))
		{
			if (r == RenderMode::List)
			{
				if (Bundlepath != nullptr || Bundlepath[0] != '\0' || Bundlepath != "")
				{
					ImGui::Text("Current Path: ");
					ImGui::SameLine();
					if (ImGui::Button(std::string(Bundlepath).substr(std::string(Bundlepath).find_last_of("\\") + 1).c_str()))
					{
						CurrentPKG = "";
						CurrentCAFF = "";
					}

					if (CurrentPKG != "")
					{
						ImGui::SameLine();
						ImGui::Text("/");
						ImGui::SameLine();
						if (ImGui::Button(CurrentPKG.c_str()))
						{
							CurrentCAFF = "";
						}

						if (CurrentCAFF != "")
						{
							ImGui::SameLine();
							ImGui::Text("/");
							ImGui::SameLine();
							if (ImGui::Button(CurrentCAFF.c_str()))
							{
								CurrentAid = "";
							}

							ImGui::Text("Search: ");
							ImGui::SameLine();

							strncpy(Searchbuf, SearchTerm.c_str(), sizeof(Searchbuf) - 1);
							ImGui::InputText("##Search", Searchbuf, 256);
							SearchTerm = Searchbuf;

							ImGui::BeginChild("Scrolling");
							ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));

							int currentcaffindex = std::stoi(CurrentCAFF.substr(CurrentCAFF.find_last_of(" ") + 1)) - 1;

							//for each chunk in VREF
							for (int i = 0; i < pkg.VREFs[currentcaffindex].ChunkNames.size(); i++)
							{
								ChunkInfo* chunk = &pkg.VREFs[currentcaffindex].ChunkInfos[i];

								//if search term is empty or chunk name contains search term
								if (SearchTerm == "" || chunk->ChunkName.find(SearchTerm) != std::string::npos)
								{
									ImGui::PushID(i);
									ImGui::BeginGroup();

									bool IsDDS;
									if (chunk->Type == FileType::DDS)
									{
										IsDDS = true;
									}
									else
									{
										IsDDS = false;
									}
									ImGui::Image(GetAssetThumbnail(chunk->ChunkName, IsDDS)->GetDescriptorSet(), ImVec2(50, 50));
									ImGui::SameLine();
									//center height
									ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (50 - ImGui::GetTextLineHeight()) / 2);
									//i.caff
									//tool tip
									if (ImGui::IsItemHovered())
									{
										ImGui::BeginTooltip();
										ImGui::Text("Chunk Name: %s", chunk->ChunkName.c_str());
										ImGui::Text("ID: %d", chunk->ID);
										ImGui::Text("VDAT Offset: %d", chunk->VDAT_Offset);
										ImGui::Text("VDAT Size: %d", chunk->VDAT_Size);
										if (chunk->HasVGPU)
										{
											ImGui::Text("VGPU Offset: %d", chunk->VGPU_Offset);
											ImGui::Text("VGPU Size: %d", chunk->VGPU_Size);
										}
										ImGui::Text("Type: %d", chunk->Type);
										ImGui::Text("Debug Data: %d", chunk->DebugData);
										ImGui::EndTooltip();
									}
									if (ImGui::Button(chunk->ChunkName.c_str())) {
										CurrentAid = chunk->ChunkName;

										std::ifstream file(pkg.path, std::ios::binary);
										//Check real file type
										FileType RealFileType = pkg::GetFileType(chunk->ChunkName, pkg::GetChunkVDATBYTES(currentcaffindex, i, pkg, file), pkg::GetChunkVGPUBYTES(currentcaffindex, i, pkg, file));
										if (RealFileType != FileType::DDS)
										{
											IsDDS = false;
											chunk->Type = RealFileType;
										}
										UpdateSelectedAid(chunk->ChunkName, pkg::GetChunkVDATBYTES(currentcaffindex, i, pkg, file), pkg::GetChunkVGPUBYTES(currentcaffindex, i, pkg, file));

										file.close();

										ImGui::OpenPopup("popup");
									}
									if (ImGui::BeginPopup("popup")) {
										CurrentAid = chunk->ChunkName;


										//add buttons to view vdat and vgpu
										if (ImGui::Button("View VDAT")) {
											std::ifstream file(pkg.path, std::ios::binary);
											HexData = pkg::GetChunkVDATBYTES(currentcaffindex, i, pkg, file);
											ShowHexWindow = true;
											file.close();
										}
										ImGui::SameLine();
										if (ImGui::Button("Export VDAT")) {
											std::ifstream file(pkg.path, std::ios::binary);
											vBYTES VDATData = pkg::GetChunkVDATBYTES(currentcaffindex, i, pkg, file);
											bool saved = Walnut::OpenFileDialog::AskSaveFile(VDATData, chunk->ChunkName + ".vdat");
										}
										ImGui::SameLine();
										if (ImGui::Button("Replace VDAT")) {
											//select Patch file of .vdat .vgpu .chunk .dds
											std::string PatchFilePath = Walnut::OpenFileDialog::OpenFile("VDAT Files\0*.vdat\0\0");
											//select NewPKGExportPath
											std::string NewPKGExportPath = "";

											std::string PatchFileExt = PatchFilePath.substr(PatchFilePath.find_last_of(".") + 1);

											ChunkType PatchFileType;
											if (PatchFileExt == "vdat")
											{
												PatchFileType = ChunkType::VDAT;
											}
											else if (PatchFileExt == "vgpu")
											{
												PatchFileType = ChunkType::VGPU;
											}
											else if (PatchFileExt == "dds")
											{
												PatchFileType = ChunkType::DDS;
											}

											//replace chunk
											pkg::ReplaceChunk(pkg, currentcaffindex, chunk->ChunkName, PatchFileType, PatchFilePath, NewPKGExportPath, true);
											reloadPKG();
											//return;
										}
										if (chunk->HasVGPU)
										{
											if (ImGui::Button("View VGPU")) {
												std::ifstream file(pkg.path, std::ios::binary);
												HexData = pkg::GetChunkVGPUBYTES(currentcaffindex, i, pkg, file);
												ShowHexWindow = true;
												file.close();
											}
											ImGui::SameLine();
											if (ImGui::Button("Export VGPU")) {
												std::ifstream file(pkg.path, std::ios::binary);
												vBYTES VGPUData = pkg::GetChunkVGPUBYTES(currentcaffindex, i, pkg, file);
												bool saved = Walnut::OpenFileDialog::AskSaveFile(VGPUData, chunk->ChunkName + ".vgpu");
											}
											ImGui::SameLine();
											if (ImGui::Button("Replace VGPU")) {
												//select Patch file of .vdat .vgpu .chunk .dds
												std::string PatchFilePath = Walnut::OpenFileDialog::OpenFile("VGPU Files\0*.vgpu\0\0");
												//select NewPKGExportPath
												std::string NewPKGExportPath = "";

												std::string PatchFileExt = PatchFilePath.substr(PatchFilePath.find_last_of(".") + 1);

												ChunkType PatchFileType;
												if (PatchFileExt == "vdat")
												{
													PatchFileType = ChunkType::VDAT;
												}
												else if (PatchFileExt == "vgpu")
												{
													PatchFileType = ChunkType::VGPU;
												}
												else if (PatchFileExt == "dds")
												{
													PatchFileType = ChunkType::DDS;
												}

												//replace chunk
												pkg::ReplaceChunk(pkg, currentcaffindex, chunk->ChunkName, PatchFileType, PatchFilePath, NewPKGExportPath, true);
												reloadPKG();
												//return;
											}
										}
										if (chunk->Type == FileType::DDS)
										{
											if (ImGui::Button("View DDS Image")) {
												std::ifstream file(pkg.path, std::ios::binary);
												//ExportChunk to temp file called temp.dds
												vBYTES DDSData = pkg::GetChunkVGPUBYTES(currentcaffindex, i, pkg, file);
												//remove first 4 vBYTES
												DDSData.erase(DDSData.begin(), DDSData.begin() + 4);
												//write to temp.dds
												std::ofstream DDSFile("Assets/temp.dds", std::ios::binary);
												DDSFile.write((char*)DDSData.data(), DDSData.size());
												DDSFile.close();
												//load temp.dds
												dds::LoadDDS("Assets/temp.dds");
												//delay
												UpdateImage = true;
												ShowRaw = false;
												ShowImageWindow = true;
												file.close();
											}
											ImGui::SameLine();
											if (ImGui::Button("Export DDS"))
											{
												std::ifstream file(pkg.path, std::ios::binary);
												vBYTES DDSData = pkg::GetChunkVGPUBYTES(currentcaffindex, i, pkg, file);
												DDSData.erase(DDSData.begin(), DDSData.begin() + 4);
												bool saved = Walnut::OpenFileDialog::AskSaveFile(DDSData, chunk->ChunkName + ".dds");
												file.close();
											}
											ImGui::SameLine();
											if (ImGui::Button("Replace DDS"))
											{
												//select Patch file of .vdat .vgpu .chunk .dds
												std::string PatchFilePath = Walnut::OpenFileDialog::OpenFile("DDS Files\0*.dds\0\0");
												//select NewPKGExportPath
												std::string NewPKGExportPath = "";
												std::string PatchFileExt = PatchFilePath.substr(PatchFilePath.find_last_of(".") + 1);
												ChunkType PatchFileType = ChunkType::VGPU;
												//replace chunk
												pkg::ReplaceChunk(pkg, currentcaffindex, chunk->ChunkName, PatchFileType, PatchFilePath, NewPKGExportPath, true, true);
												reloadPKG();
												//return;
											}
										}
										if (true) {//chunk->Type == FileType::RawImage
											if (ImGui::Button("View Raw Image")) {
												std::ifstream file(pkg.path, std::ios::binary);
												//Get both VDAT and VGPU
												vBYTES VDATData = pkg::GetChunkVDATBYTES(currentcaffindex, i, pkg, file);
												vBYTES VGPUData = pkg::GetChunkVGPUBYTES(currentcaffindex, i, pkg, file);

												raw::LoadRAW(VDATData, VGPUData, pkg.IsBigEndian);

												ShowRaw = true;
												ShowImageWindow = true;

												file.close();
											}
										}
										if (chunk->Type == FileType::Model)
										{
											
											if (ImGui::Button("Export Model (WIP)")) {
												std::ifstream file(pkg.path, std::ios::binary);
												vBYTES VDATData = pkg::GetChunkVDATBYTES(currentcaffindex, i, pkg, file);
												vBYTES VGPUData = pkg::GetChunkVGPUBYTES(currentcaffindex, i, pkg, file);
												model::ExportModel(VDATData, VGPUData, "", pkg.IsBigEndian);
											}
										}
										ImGui::EndPopup();
									}
									//ImGui::OpenPopupOnItemClick("popup", ImGuiPopupFlags_MouseButtonRight);
									ImGui::EndGroup();
									ImGui::PopID();
								}
							}

							ImGui::PopStyleVar();

							ImGui::EndChild();
						}

						//No CAFF Selected
						else {
							//add scrollable area
							ImGui::BeginChild("Scrolling");

							//add padding
							ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));

							//To test add 10 dummy files
							for (int i = 0; i < pkg.CAFFCount; i++)
							{
								ImGui::PushID(i);
								ImGui::BeginGroup();
								//ImGui::ImageButton((ImTextureID)0, ImVec2(50, 50));
								ImGui::Image(m_Unknown_Thumbnail.GetDescriptorSet(), ImVec2(50, 50));
								ImGui::SameLine();
								//center height
								ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (50 - ImGui::GetTextLineHeight()) / 2);
								//i.caff
								//tool tip
								if (ImGui::IsItemHovered())
								{
									ImGui::BeginTooltip();
									ImGui::Text("CAFF %d", i + 1);
									ImGui::Text("PKG Offset: %d", pkg.CAFF_Infos[i].Offset);
									ImGui::Text("PKG Size: %d", pkg.CAFF_Infos[i].Size);
									ImGui::EndTooltip();
								}

								if (ImGui::Button(std::string("CAFF " + std::to_string(i + 1)).c_str())) {
									ImGui::OpenPopup("caffpopup");
								}
								if (ImGui::BeginPopup("caffpopup")) {
									if (ImGui::Button("Open")) {
										CurrentCAFF = "CAFF " + std::to_string(i + 1);
									}
									if (ImGui::Button("Export VREF")) {
										std::ifstream file11(pkg.path, std::ios::binary);
										vBYTES VREFData = pkg::GetVREFBYTES(file11, pkg.CAFFs[i]);
										bool saved = Walnut::OpenFileDialog::AskSaveFile(VREFData, "CAFF " + std::to_string(i + 1) + ".vref");
										file11.close();
									}
									ImGui::SameLine();
									if (ImGui::Button("Export VDAT")) {
										std::ifstream file11(pkg.path, std::ios::binary);
										vBYTES VREFData = pkg::GetVDATBYTES(i, pkg, file11);
										bool saved = Walnut::OpenFileDialog::AskSaveFile(VREFData, "CAFF " + std::to_string(i + 1) + ".vref");
										file11.close();
									}
									ImGui::SameLine();
									if (ImGui::Button("Export VGPU"))
									{
										std::ifstream file11(pkg.path, std::ios::binary);
										vBYTES VREFData = pkg::GetVGPUBYTES(i, pkg, file11);
										bool saved = Walnut::OpenFileDialog::AskSaveFile(VREFData, "CAFF " + std::to_string(i + 1) + ".vref");
										file11.close();
									}

									ImGui::EndPopup();
								}

								ImGui::EndGroup();
								ImGui::PopID();
							}

							ImGui::PopStyleVar();

							ImGui::EndChild();
						}
					}

					//No PKG selected
					else {
						//add scrollable area
						ImGui::BeginChild("Scrolling");

						//add padding
						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));

						//for each file in directory
						if (!FoundFiles)
						{
							files = Walnut::OpenFileDialog::GetFilesInDirectory(Bundlepath);

							for (int i = 0; i < files.size(); i++)
							{
								FileNames.push_back(files[i].substr(files[i].find_last_of("\\") + 1));
							}
							FoundFiles = true;
						}

						for (int i = 0; i < files.size(); i++)
						{
							ImGui::BeginGroup();
							ImGui::PushID(i);

							//ImGui::ImageButton((ImTextureID)0, ImVec2(50, 50));
							if (ImGui::ImageButton(m_Unknown_Thumbnail.GetDescriptorSet(), ImVec2(50, 50))) {
								ImGui::OpenPopup("pkgpopup");
							}

							ImGui::SameLine();
							//center height
							ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (50 - ImGui::GetTextLineHeight()) / 2);
							if (ImGui::Button(FileNames[i].c_str()))
							{
								ImGui::OpenPopup("pkgpopup");
							}
							if (ImGui::BeginPopup("pkgpopup")) {
								if (ImGui::Button("Open")) {
									CurrentPKG = FileNames[i];
									std::cout << "Selected: " << CurrentPKG << std::endl;
									//read pkg
									pkg = pkg::ReadPKG(files[i]);
									std::cout << "Version: " << pkg.Version << std::endl;
									std::cout << "Big Endian: " << pkg.IsBigEndian << std::endl;
									std::cout << "CAFF Count: " << pkg.CAFFCount << std::endl;
								}
								ImGui::SameLine();
								if (std::filesystem::exists("Backups\\PackageBundles\\" + files[i].substr(pkg.path.find_last_of("\\") + 1)))
								{
									if (ImGui::Button("Restore")) {
										std::filesystem::remove(files[i]);
										std::filesystem::copy("Backups\\PackageBundles\\" + files[i].substr(pkg.path.find_last_of("\\") + 1), files[i]);
										std::cout << "Restored: " << files[i] << std::endl;
										ImGui::CloseCurrentPopup();
									}
								}

								ImGui::EndPopup();
							}

							ImGui::PopID();
							ImGui::EndGroup();
						}

						ImGui::PopStyleVar();

						ImGui::EndChild();
					}
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

		if (ShowImageWindow) {
			RenderImage("Assets/temp.PNG");
		}
		aidEditor.Render();
	}

	void RenderImage(std::string path)
	{
		if (ShowRaw)
		{
			ImGui::Begin("Image Viewer");
			ImGui::Image(raw::m_FinalImage->GetDescriptorSet(), ImVec2(ImGui::GetWindowSize().y, ImGui::GetWindowSize().y));
			ImGui::End();
			return;
		}
		else {
			Walnut::Image img = Walnut::Image(path);
			ImGui::Begin("Image Viewer");
			ImGui::Image(img.GetDescriptorSet(), ImVec2(ImGui::GetWindowSize().y, ImGui::GetWindowSize().y));
			ImGui::End();
		}
	}
};