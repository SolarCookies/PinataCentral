#pragma once
#include <string>
#include <iostream>
#include <filesystem>
#include "PKG/pkg.h"


namespace debug_pack {

	std::vector<vBYTES> CAFFS;

	void read(std::string path) {
		//check if file exists
		if (!std::filesystem::exists(path)) {
			std::cout << "File does not exist: " << path << std::endl;
			return;
		}
		else {
			std::cout << "File exists: " << path << std::endl;
			CAFFS.clear();
			// Read the file into a buffer
			std::ifstream file(path, std::ios::binary | std::ios::ate);
			if (!file) {
				std::cout << "Failed to open file: " << path << std::endl;
				return;
			}
			std::streamsize size = file.tellg();
			file.seekg(0, std::ios::beg);

			std::vector<uint8_t> buffer(size);
			if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
				std::cout << "Failed to read file: " << path << std::endl;
				return;
			}

			// Search for "CAFF" markers
			const std::vector<uint8_t> marker = { 0x43, 0x41, 0x46, 0x46 }; // "CAFF"
			size_t pos = 0;
			std::vector<size_t> marker_positions;

			// Find all marker positions
			while (pos + marker.size() <= buffer.size()) {
				if (std::equal(marker.begin(), marker.end(), buffer.begin() + pos)) {
					marker_positions.push_back(pos);
				}
				++pos;
			}

			// Extract segments between markers
			for (size_t i = 0; i < marker_positions.size(); ++i) {
				size_t start = marker_positions[i];
				size_t end = (i + 1 < marker_positions.size()) ? marker_positions[i + 1] : buffer.size();
				vBYTES segment(buffer.begin() + start, buffer.begin() + end);
				CAFFS.push_back(segment);
			}
			int caffNumber = CAFFS.size() - 1;
			std::cout << "Found " << CAFFS.size() << " CAFF segments." << std::endl;
			//Debug pack is Big Endian initially with the CAFF header, however the vref is little endian
			CAFF caff = caff::Read_Header(CAFFS[caffNumber]);
			std::cout << "CAFF " << caffNumber << " Version: " << caff.CAFF_Version << std::endl;
			std::cout << "CAFF " << caffNumber << " ChunkCount: " << caff.ChunkCount << std::endl;
			std::cout << "CAFF " << caffNumber << " VRef Offset: " << caff.VREF_Offset << std::endl;
			std::cout << "CAFF " << caffNumber << " VRef Uncompressed Size: " << caff.VREF_Uncompressed_Size << std::endl;
			std::cout << "CAFF " << caffNumber << " VRef Compressed Size: " << caff.VREF_Compressed_Size << std::endl;
			std::cout << "CAFF " << caffNumber << " VLUT Offset: " << caff.VLUT_Offset << std::endl;
			std::cout << "CAFF " << caffNumber << " VLUT Uncompressed Size: " << caff.VLUT_Uncompressed_Size << std::endl;
			std::cout << "CAFF " << caffNumber << " VLUT Compressed Size: " << caff.VLUT_Compressed_Size << std::endl;

			vBYTES VREFB = caff::Get_VREF(CAFFS[caffNumber], caff);

			//Debug pack is Big Endian initially with the CAFF header, however the vref is little endian
			VREF vref = caff::Read_VREF(VREFB, caff, caff.IsBigEndian);

			std::cout << "VREF " << " VGPU Offset: " << vref.VGPU_Offset << std::endl;
			std::cout << "VREF " << " VGPU Compressed Size: " << vref.VGPU_Compressed_Size << std::endl;
			std::cout << "VREF " << " VGPU Uncompressed Size: " << vref.VGPU_Uncompressed_Size << std::endl;
			std::cout << "VREF " << " VDAT Offset: " << vref.VDAT_Offset << std::endl;
			std::cout << "VREF " << " VDAT Compressed Size: " << vref.VDAT_Compressed_Size << std::endl;
			std::cout << "VREF " << " VDAT Uncompressed Size: " << vref.VDAT_Uncompressed_Size << std::endl;
			std::cout << "VREF " << " NameBlockOffset: " << vref.NameBlockOffset << std::endl;
			std::cout << "VREF " << " InfoBlockOffset: " << vref.InfoBlockOffset << std::endl;
			std::cout << "VREF " << " Chunk Count: " << vref.ChunkInfos.size() << std::endl;

			std::cout << " " << std::endl;

			for (size_t i = 0; i < vref.ChunkInfos.size(); ++i) {
				const ChunkInfo& chunk = vref.ChunkInfos[i];
				std::cout << "Chunk " << i << ": " << chunk.ChunkName << std::endl;
				std::cout << "  ID: " << chunk.ID << std::endl;
				std::cout << "  VDAT Offset: " << chunk.VDAT_Offset << std::endl;
				std::cout << "  VDAT Size: " << chunk.VDAT_Size << std::endl;
				std::cout << "  VGPU Offset: " << chunk.VGPU_Offset << std::endl;
				std::cout << "  VGPU Size: " << chunk.VGPU_Size << std::endl;
			}

			//output vref file and full caff file to desktop
			std::string desktopPath = std::filesystem::path(std::getenv("USERPROFILE")).append("Desktop").string();
			std::string vrefPath = desktopPath + "\\VREF_" + std::to_string(caffNumber) + ".vref";
			std::ofstream vrefFile(vrefPath, std::ios::binary);
			if (vrefFile) {
				vrefFile.write(reinterpret_cast<const char*>(VREFB.data()), VREFB.size());
				vrefFile.close();
				std::cout << "VREF file saved to: " << vrefPath << std::endl;
			}
			else {
				std::cout << "Failed to save VREF file to: " << vrefPath << std::endl;
			}
			std::string caffPath = desktopPath + "\\CAFF_" + std::to_string(caffNumber) + ".caff";
			std::ofstream caffFile(caffPath, std::ios::binary);
			if (caffFile) {
				caffFile.write(reinterpret_cast<const char*>(CAFFS[caffNumber].data()), CAFFS[caffNumber].size());
				caffFile.close();
				std::cout << "CAFF file saved to: " << caffPath << std::endl;
			}
			else {
				std::cout << "Failed to save CAFF file to: " << caffPath << std::endl;
			}

		}
	}


}
