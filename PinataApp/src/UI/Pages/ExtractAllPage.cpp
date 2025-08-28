#include "ExtractAllPage.h"
#include "../../GlobalSettings.h"

void ExtractPage::render(GUI& gui)
{
	LoadSettings();
	ImGui::Begin("Welcome");
	//center the text
	ImGui::SetWindowFontScale(1.0f);
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("With this menu you can extract all the files inside the .pkg files").x) * 0.5f);
	ImGui::Text("With this menu you can extract all the files inside the .pkg files");
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetTextLineHeightWithSpacing());
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Please select a export path for the files").x) * 0.5f);
	ImGui::Text("Please select a export path for the files");
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Select Path").x) * 0.5f);
	//Add button to select the path
	if (ImGui::Button("Select Path")) {
		Path = Walnut::OpenFileDialog::OpenFolder();
	}
	// Display the selected path
	if (Path.empty()) {
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(std::string("No path selected.").c_str()).x) * 0.5f);
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No path selected.");
		goto End;
	}

	//Check to make sure the path is empty
	if (!std::filesystem::is_empty(Path)) {
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(std::string("The selected path is not empty. You may want to select a empty folder.").c_str()).x) * 0.5f);
		ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "The selected path is not empty. You may want to select a empty folder.");
	}
	else {
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(std::string("Selected Path: " + Path).c_str()).x) * 0.5f);
		ImGui::Text(("Selected Path: " + Path).c_str());
	}
	//Add a text input for a filter for the file, any file with this string in the name will be extracted
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetTextLineHeightWithSpacing());
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Filter (optional)").x) * 0.5f);
	ImGui::Text("Filter (optional)");
	ImGui::SetCursorPosX(((ImGui::GetWindowWidth() - ImGui::CalcTextSize(filter).x) * 0.5f)*0.35f);
	ImGui::InputText("##filter", filter, sizeof(filter)); // Pass the char array and its size

	
	//If the path is valid, the rest will run
	// Add a button to continue and make it green
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetTextLineHeightWithSpacing());
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Extract").x) * 0.5f);
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.5f, 0.0f, 1.0f));
	if (ImGui::Button("Extract")) {
		PKGIndex = 0; //start extracting from the first .pkg file
	}
	ImGui::PopStyleColor();

	if (PKGIndex != -1) {
		//extract PKGIndex file
		//Draw a progress bar with the percentage of the extraction PKGIndex/TotalPKGs
		//get number of .pkg files in the input path
		auto pkgFiles = std::filesystem::recursive_directory_iterator(PackagesDicrectory);
		std::vector<std::filesystem::path> pkgFilePaths;
		for (const auto& entry : pkgFiles) {
			if (entry.path().extension() == ".pkg") {
				pkgFilePaths.push_back(entry.path());
			}
		}
		int totalPKGs = pkgFilePaths.size();
		float progress = (float)(PKGIndex + 1) / (float)totalPKGs;

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetTextLineHeightWithSpacing());
		ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f));
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetTextLineHeightWithSpacing());
		ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(("Extracting file " + std::to_string(PKGIndex + 1) + " of " + std::to_string(totalPKGs)).c_str()).x) * 0.5f);
		ImGui::Text(("Extracting file " + std::to_string(PKGIndex + 1) + " of " + std::to_string(totalPKGs)).c_str());

		//Extract the file
		PKG CurrentPkg = pkg::ReadPKG(pkgFilePaths[PKGIndex].string());
		//open pkg file 
		std::ifstream pkgfile(CurrentPkg.path, std::ios::binary);
		if (!pkgfile.is_open()) {
			return;
		}

		int CaffIndex = 0;
		for(CAFF& CurrentCAFF : CurrentPkg.CAFFs) {
			VREF& CurrentVREF = CurrentPkg.VREFs[CaffIndex];
			CaffIndex++;

			
			vBYTES CAFFBytes = pkg::GetCAFFBytes(pkgfile, CurrentCAFF);
			vBYTES FullVDAT = caff::Get_VDAT(CAFFBytes, CurrentCAFF, CurrentVREF);
			vBYTES FullVGPU = caff::Get_VGPU(CAFFBytes, CurrentCAFF, CurrentVREF);

			int chunkIndex = 0;
			//CurrentVREF.ChunkInfos
			for(const ChunkInfo& currentChunk : CurrentVREF.ChunkInfos) {
				
				//If the filter is not empty, check if the currentChunk.Name contains the filter string
				if (strlen(filter) > 0) {
					std::string filterStr(filter);
					if (currentChunk.ChunkName.find(filterStr) == std::string::npos) {
						continue; //skip this file
					}
				}
				chunkIndex++;
				//get everything before the first "," in the string
				std::string chunkNameFull = currentChunk.ChunkName;
				size_t commaPos = chunkNameFull.find(',');
				std::string chunkName = (commaPos != std::string::npos) ? chunkNameFull.substr(0, commaPos) : chunkNameFull;
				
				std::string PathName = chunkName + "_" + std::to_string(chunkIndex); //add the chunk ID to the name to make it unique

				//Extract the file
				//Path/PKGNumber/CAFFNumber/Chunk Name/ Chunk .vdat/.vgpu
				//get pkg name from pkg.path without the extension
				std::string pkgName = std::filesystem::path(CurrentPkg.path).stem().string();

				std::filesystem::path outputPath = std::filesystem::path(Path) / ("PKG_" + pkgName) / ("CAFF_" + std::to_string(CaffIndex)) / PathName;
				std::filesystem::create_directories(outputPath.parent_path());
				std::filesystem::path vdatPath = outputPath / (chunkName + ".vdat");
				std::filesystem::path vgpuPath = outputPath / (chunkName + ".vgpu");

				//std::cout << "Extracting: " << vdatPath.string() << std::endl;

				//create path if it doesn't exist
				if (!std::filesystem::exists(outputPath)) {
					std::filesystem::create_directories(outputPath);
				}



				vBYTES chunkVDat;
				chunkVDat.resize(currentChunk.VDAT_Size);
				std::memcpy(chunkVDat.data(), FullVDAT.data() + currentChunk.VDAT_Offset, currentChunk.VDAT_Size);

				//Write the .vdat file
				std::ofstream vdatFile(vdatPath, std::ios::binary);
				if (vdatFile.is_open()) {
					vdatFile.write((char*)chunkVDat.data(), chunkVDat.size());
					vdatFile.close();
				}
				//Write the .vgpu file if it exists
				if (currentChunk.VGPU_Size > 0) {
					vBYTES chunkVGpu;
					chunkVGpu.resize(currentChunk.VGPU_Size);
					std::memcpy(chunkVGpu.data(), FullVGPU.data() + currentChunk.VGPU_Offset, currentChunk.VGPU_Size);
					std::ofstream vgpuFile(vgpuPath, std::ios::binary);
					if (vgpuFile.is_open()) {
						vgpuFile.write((char*)chunkVGpu.data(), chunkVGpu.size());
						vgpuFile.close();
					}
					chunkVGpu.clear();
					chunkVGpu.shrink_to_fit();
				}
				//cleanup
				chunkVDat.clear();
				chunkVDat.shrink_to_fit();
				
				
			}

			CAFFBytes.clear();
			CAFFBytes.shrink_to_fit();
			
			FullVDAT.clear();
			FullVDAT.shrink_to_fit();
			
			FullVGPU.clear();
			FullVGPU.shrink_to_fit();
			
			pkgfile.close();
		}

		PKGIndex++;

		if (PKGIndex >= totalPKGs) {
			PKGIndex = -1; //reset to -1 to indicate we're done
		}

	}

End:
	ImGui::End();
};
