#pragma once
#include <string>
#include <iostream>
#include <filesystem>
#include "PKG/pkg.h"

namespace Bundle {

	BYTES CAFFBYTES;
	CAFF caff;

	void read(std::string path) {
		//check if file exists
		if (!std::filesystem::exists(path)) {
			std::cout << "File does not exist: " << path << std::endl;
			return;
		}
		else {
			std::cout << "File exists: " << path << std::endl;
			CAFFBYTES.clear();
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
			CAFFBYTES = BYTES(buffer.begin(), buffer.end());

			//Bundles are little endian
			caff = caff::Read_Header(CAFFBYTES);
			std::cout << "CAFF " << 1 << " Version: " << caff.CAFF_Version << std::endl;
			std::cout << "CAFF " << 1 << " ChunkCount: " << caff.ChunkCount << std::endl;
			std::cout << "CAFF " << 1 << " VRef Offset: " << caff.VREF_Offset << std::endl;
			std::cout << "CAFF " << 1 << " VRef Uncompressed Size: " << caff.VREF_Uncompressed_Size << std::endl;
			std::cout << "CAFF " << 1 << " VRef Compressed Size: " << caff.VREF_Compressed_Size << std::endl;
			std::cout << "CAFF " << 1 << " VLUT Offset: " << caff.VLUT_Offset << std::endl;
			std::cout << "CAFF " << 1 << " VLUT Uncompressed Size: " << caff.VLUT_Uncompressed_Size << std::endl;
			std::cout << "CAFF " << 1 << " VLUT Compressed Size: " << caff.VLUT_Compressed_Size << std::endl;

			BYTES VREFB = caff::Get_VREF(CAFFBYTES, caff);

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



		}
	}


}

