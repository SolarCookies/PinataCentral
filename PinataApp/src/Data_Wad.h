#pragma once
#include <string>
#include <iostream>
#include <filesystem>

struct WadEntry {
	uint32_t Checksum;
	uint32_t Offset;
};

namespace Data_Wad {

	void read(std::string path) {
		//check if file exists
		if (!std::filesystem::exists(path)) {
			std::cout << "File does not exist: " << path << std::endl;
			return;
		}
		else {
			std::cout << "File exists: " << path << std::endl;

			// read the 77 Wad Entries beginning at offset 16
			std::ifstream file(path, std::ios::binary);
			if (!file) {
				std::cout << "Failed to open file: " << path << std::endl;
				return;
			}

			// Seek to offset 16
			file.seekg(16, std::ios::beg);

			const size_t entryCount = 77;
			WadEntry entries[entryCount];

			// Read 77 WadEntry structs
			file.read(reinterpret_cast<char*>(entries), sizeof(WadEntry) * entryCount);

			if (!file) {
				std::cout << "Failed to read Wad entries." << std::endl;
				return;
			}

			// Optionally, print out the entries for verification
			for (size_t i = 0; i < entryCount; ++i) {
				std::cout << "Entry " << i
					<< " - Checksum: " << entries[i].Checksum
					<< ", Offset: " << entries[i].Offset << std::endl;

				BYTES entryData;
				// Read the data for each entry by using the offset of the next entry or the end of the file
				if (i < entryCount - 1) {
					file.seekg(entries[i].Offset, std::ios::beg);
					size_t nextOffset = entries[i + 1].Offset;
					size_t size = nextOffset - entries[i].Offset;
					entryData.resize(size);
					file.read(reinterpret_cast<char*>(entryData.data()), size);
				}
				else {
					// For the last entry, read until the end of the file
					file.seekg(entries[i].Offset, std::ios::beg);
					entryData.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
				}
				//write
				std::string entryPath = "Entry_" + std::to_string(i) + ".bin";
				std::ofstream entryFile(entryPath, std::ios::binary);
				if (entryFile) {
					entryFile.write(reinterpret_cast<const char*>(entryData.data()), entryData.size());
					entryFile.close();
					std::cout << "Wrote entry " << i << " to " << entryPath << std::endl;
				}
				else {
					std::cout << "Failed to write entry " << i << " to file." << std::endl;
				}
			}
		}
	}

}
