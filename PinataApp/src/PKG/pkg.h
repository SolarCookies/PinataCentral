#pragma once
#include <stdint.h>
#include <vector>
#include <string>
#include "../Utils/ZLibHelpers.h"
#include "../Utils/OpenFileDialog.h"
#include <iostream>

#include "Walnut/Timer.h"

#include <omp.h>

//FileTypeEnum
enum class FileType
{
	Unknown,
	DDS,
	RawImage
};

enum class ChunkType
{
	Unknown,
	VDAT,
	VGPU,
	DDS
};

struct CAFF_Info {
	std::string PKGpath = "";
	uint32_t Offset = -0;
	uint32_t Size = 0;
	uint32_t Number = 0;
	uint32_t Unknown = 0;
};

struct CAFF {
	CAFF_Info CAFF_Info;
	std::string CAFF_Version = "";
	uint32_t ChunkCount = 0;
	uint32_t ChunkSpreadCount = 0;
	uint32_t VREF_Offset = 0;
	uint32_t VREF_Uncompressed_Size = 0;
	uint32_t VREF_Compressed_Size = 0;
	uint32_t VLUT_Offset = 0;
	uint32_t VLUT_Uncompressed_Size = 0;
	uint32_t VLUT_Compressed_Size = 0;
};

struct ChunkInfoOffsets {
	uint32_t VDAT_Offset_Location = 0;
	uint32_t VDAT_Size_Location = 0;
	uint32_t VGPU_Offset_Location = 0;
	uint32_t VGPU_Size_Location = 0;
};

struct ChunkInfo {
	std::string ChunkName = "";
	uint32_t ID = 0;
	uint32_t VDAT_Offset = 0;
	uint32_t VDAT_Size = 0;
	BYTE VDAT_File_Data_1 = NULL;
	BYTE VDAT_File_Data_2 = NULL;
	bool HasVGPU = false;
	uint32_t VGPU_Offset = 0;
	uint32_t VGPU_Size = 0;
	BYTE VGPU_File_Data_1 = NULL;
	BYTE VGPU_File_Data_2 = NULL;
	ChunkInfoOffsets OffsetLocations;
	FileType Type = FileType::Unknown;
	BYTE DebugData = NULL; //Used for debugging when checking if a offset is correct
};

struct VREF {
	CAFF_Info CAFF;
	std::vector<std::string> ChunkNames;
	uint32_t VGPU_Offset = 0;
	uint32_t VGPU_Compressed_Size = 0;
	uint32_t VGPU_Uncompressed_Size = 0;
	uint32_t VDAT_Offset = 0;
	uint32_t VDAT_Compressed_Size = 0;
	uint32_t VDAT_Uncompressed_Size = 0;
	std::vector<ChunkInfo> ChunkInfos;

};

struct PKG {
	std::string path = "";
	uint32_t Version = 0;
	bool IsBigEndian = false;
	uint32_t CAFFCount = 0;
	std::vector<CAFF_Info> CAFF_Infos;
	std::vector<CAFF> CAFFs;
	std::vector<VREF> VREFs;
};

struct Streams {
	BYTES VREF;
	BYTES VDAT;
	BYTES VGPU;
};


namespace pkg {

	inline static BYTES GetVREFBYTES(std::ifstream& file, CAFF C) {

		//Save current position
		uint32_t CurrentPosition = file.tellg();
		//Goto vref offset
		Walnut::OpenFileDialog::SeekBeg(file, C.CAFF_Info.Offset + C.VREF_Offset);

		BYTES COMPRESSEDVREF = Walnut::OpenFileDialog::Read_Bytes(file, C.VREF_Compressed_Size);

		//Restore position
		Walnut::OpenFileDialog::SeekBeg(file, CurrentPosition);

		return Zlib::DecompressData(COMPRESSEDVREF, C.VREF_Uncompressed_Size);

	}

	inline static BYTES GetVDATBYTES(uint32_t CAFFNumber, PKG package, std::ifstream& file) {
		return Zlib::DecompressData(Walnut::OpenFileDialog::Read_Bytes(file, package.VREFs[CAFFNumber].VDAT_Offset + package.CAFF_Infos[CAFFNumber].Offset, package.VREFs[CAFFNumber].VDAT_Compressed_Size), package.VREFs[CAFFNumber].VDAT_Uncompressed_Size);
	}

	inline static BYTES GetChunkVDATBYTES(uint32_t CAFFNumber, uint32_t ChunkNumber, PKG package, std::ifstream& file) {
		return Walnut::OpenFileDialog::CopyBytes(GetVDATBYTES(CAFFNumber, package, file), package.VREFs[CAFFNumber].ChunkInfos[ChunkNumber].VDAT_Offset, package.VREFs[CAFFNumber].ChunkInfos[ChunkNumber].VDAT_Size);
	}

	inline static BYTES GetVGPUBYTES(uint32_t CAFFNumber, PKG package, std::ifstream& file) {
		return Zlib::DecompressData(Walnut::OpenFileDialog::Read_Bytes(file, package.VREFs[CAFFNumber].VGPU_Offset + package.CAFF_Infos[CAFFNumber].Offset, package.VREFs[CAFFNumber].VGPU_Compressed_Size), package.VREFs[CAFFNumber].VGPU_Uncompressed_Size);
	}

	inline static BYTES GetChunkVGPUBYTES(uint32_t CAFFNumber, uint32_t ChunkNumber, PKG package, std::ifstream& file) {
		return Walnut::OpenFileDialog::CopyBytes(GetVGPUBYTES(CAFFNumber, package, file), package.VREFs[CAFFNumber].ChunkInfos[ChunkNumber].VGPU_Offset, package.VREFs[CAFFNumber].ChunkInfos[ChunkNumber].VGPU_Size);
	}

	FileType GetFileType(std::string ChunkName, BYTES& VDAT, BYTES& VGPU)
	{
		//if the first 4 bytes are "2E 64 64 64" then return dds
		if (VDAT.size() > 25)
		{
			if (VDAT[25] == 0x60 || VDAT[25] == 0x10 || VDAT[25] == 0x30 || VDAT[25] == 0xB0)
			{
				return FileType::DDS;
			}
			if (VDAT[26] == 0x40)//26 is TIP Raw
			{
				return FileType::RawImage;
			}
		}
		return FileType::Unknown;
	}

	inline static bool VREFTypeCheck(BYTES& VREF) {
		bool isData = true;
		bool isGPU = true;
		bool isStream = true;

		//0x02000000
		BYTES DataType = { BYTE(0x02), BYTE(0x00), BYTE(0x00), BYTE(0x00) };
		//0x05010000
		BYTES GPUType = { BYTE(0x05), BYTE(0x01), BYTE(0x00), BYTE(0x00) };
		//0x0C000000
		BYTES StreamType = { BYTE(0x0C), BYTE(0x00), BYTE(0x00), BYTE(0x00) };

		//Get TYPE from VREF (At offset 37, 4 bytes)
		BYTES TYPE = { VREF[37], VREF[38], VREF[39], VREF[40] };

		for (BYTE b : TYPE) {
			int current_index = 0;
			if (b != DataType[current_index]) {
				isData = false;
			}
			if (b != GPUType[current_index]) {
				isGPU = false;
			}
			if (b != StreamType[current_index]) {
				isStream = false;
			}
			current_index++;
		}

		if (isData) {
			return true;
		}
		else if (isGPU) {
			return true;
		}
		else if (isStream) {
			return true;
		}
		else {
			return true; //enabled for testing
		}
	}

	inline static PKG ReadPKG(std::string path) {
		PKG pkg;
		BYTES VDAT;
		BYTES VGPU;
		pkg.path = path;

		std::ifstream file(path, std::ios::binary);
		if (!file.is_open()) {
			std::cout << "Failed to open file: " << path << std::endl;
			return pkg;
		}

		//Read Version (4 bytes) If version is > 3 then the file is big endian
		BYTES Version = Walnut::OpenFileDialog::Read_Bytes(file, 4);
		uint32_t VersionInt = Zlib::ConvertBytesToInt(Version, false);
		pkg.Version = VersionInt;
		if (VersionInt > 3) {
			pkg.IsBigEndian = true;
			VersionInt = Zlib::ConvertBytesToInt(Version, true);
			pkg.Version = VersionInt;
		}

		//Read CAFF Count (next 4 bytes)
		BYTES CAFFCount = Walnut::OpenFileDialog::Read_Bytes(file, 4);
		pkg.CAFFCount = Zlib::ConvertBytesToInt(CAFFCount, pkg.IsBigEndian);

		//Read CAFF Info
		for (int i = 0; i < pkg.CAFFCount; i++) {
			CAFF_Info caff_info;
			//Read CAFF Info
			caff_info.PKGpath = path;
			caff_info.Number = i+1;
			caff_info.Unknown = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::Read_Bytes(file, 4), pkg.IsBigEndian);
			caff_info.Offset = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::Read_Bytes(file, 4), pkg.IsBigEndian);
			caff_info.Size = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::Read_Bytes(file, 4), pkg.IsBigEndian);
			pkg.CAFF_Infos.push_back(caff_info);
		}

		//Read CAFF Headers
		for (CAFF_Info C : pkg.CAFF_Infos) {
			//Seek offset
			Walnut::OpenFileDialog::SeekBeg(file, C.Offset);
			CAFF caff;
			caff.CAFF_Info = C;
			//CAFF Version (17 bytes)
			BYTES CAFF_Version_B = Walnut::OpenFileDialog::Read_Bytes(file, 17);
			std::string CAFF_Version = "";
			CAFF_Version = Zlib::ConvertBytesToString(CAFF_Version_B);
			caff.CAFF_Version = CAFF_Version;

			std::cout << "CAFF Version: " << CAFF_Version << std::endl;

			//forword 3 bytes of null data
			Walnut::OpenFileDialog::SeekCur(file, 3);

			//VREF Offset (4 bytes)
			caff.VREF_Offset = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::Read_Bytes(file, 4), pkg.IsBigEndian);

			//Skip 4 unknown bytes
			Walnut::OpenFileDialog::SeekCur(file, 4);

			//Chunk Amount (4 bytes)
			caff.ChunkCount = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::Read_Bytes(file, 4), pkg.IsBigEndian);

			std::cout << "Chunk Count: " << caff.ChunkCount << std::endl;
			
			//chunk spread count (4 bytes)
			caff.ChunkSpreadCount = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::Read_Bytes(file, 4), pkg.IsBigEndian);

			//skip 44 bytes
			Walnut::OpenFileDialog::SeekCur(file, 44);

			//VRef Uncompressed Size (4 bytes)
			caff.VREF_Uncompressed_Size = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::Read_Bytes(file, 4), pkg.IsBigEndian);
			
			//skip 12 bytes
			Walnut::OpenFileDialog::SeekCur(file, 12);

			//VRef Compressed Size (4 bytes)
			caff.VREF_Compressed_Size = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::Read_Bytes(file, 4), pkg.IsBigEndian);

			//VLUT Uncompressed Size (4 bytes)
			caff.VLUT_Uncompressed_Size = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::Read_Bytes(file, 4), pkg.IsBigEndian);

			//skip 12 bytes

			Walnut::OpenFileDialog::SeekCur(file, 12);

			//VLUT Compressed Size (4 bytes)
			caff.VLUT_Compressed_Size = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::Read_Bytes(file, 4), pkg.IsBigEndian);

			//VLUT Offset is VRef Offset + VRef Compressed Size
			caff.VLUT_Offset = caff.VREF_Offset + caff.VREF_Compressed_Size;

			pkg.CAFFs.push_back(caff);
		}

		//Read VREFs
		for (CAFF C : pkg.CAFFs) {

			VREF VREF;

			BYTES VREF_Uncompressed = GetVREFBYTES(file, C);

			//start at first value
			uint32_t offset = 9;

			VREF.VDAT_Uncompressed_Size = Zlib::ConvertBytesToInt(VREF_Uncompressed, offset, pkg.IsBigEndian);

			offset += 20;

			//VDAT Compressed Size (4 bytes)
			VREF.VDAT_Compressed_Size = Zlib::ConvertBytesToInt(VREF_Uncompressed, offset, pkg.IsBigEndian);


			offset += 20;

			bool VREFType = VREFTypeCheck(VREF_Uncompressed);

			offset = 42;

			//VGPU Uncompressed Size (4 bytes)
			VREF.VGPU_Uncompressed_Size = Zlib::ConvertBytesToInt(VREF_Uncompressed, offset, pkg.IsBigEndian);

			offset += 20;

			//VGPU Compressed Size (4 bytes)
			VREF.VGPU_Compressed_Size = Zlib::ConvertBytesToInt(VREF_Uncompressed, offset, pkg.IsBigEndian);

			offset = 81;

			//VLUT_OFFSET + VLUT_Compressed_Size
			VREF.VDAT_Offset = C.VLUT_Offset + C.VLUT_Compressed_Size;

			//VDAT_OFFSET + VDAT_Compressed_Size
			VREF.VGPU_Offset = VREF.VDAT_Offset + VREF.VDAT_Compressed_Size;

			//for each chunk
			for (int i = 0; i < (C.ChunkCount - 1); i++) {


				uint32_t ChunkNameoffset;
				uint32_t NameSize;

				if (i == (C.ChunkCount - 1)) {
					ChunkNameoffset = Zlib::ConvertBytesToInt(VREF_Uncompressed, offset, pkg.IsBigEndian);
					offset = 77;
					uint32_t NameBlockSize = Zlib::ConvertBytesToInt(VREF_Uncompressed, offset, pkg.IsBigEndian);
					NameSize = (NameBlockSize + (81 + (C.ChunkCount * 4))) - (ChunkNameoffset + (81 + (C.ChunkCount * 4)));
					BYTES ChunkName = Walnut::OpenFileDialog::CopyBytes(VREF_Uncompressed, ChunkNameoffset + (81 + (C.ChunkCount * 4)), NameSize);
					offset += 4;
					VREF.ChunkNames.push_back(Zlib::ConvertBytesToString(ChunkName));

					continue;

				}
				
				ChunkNameoffset = Zlib::ConvertBytesToInt(VREF_Uncompressed, offset, pkg.IsBigEndian);

				uint32_t NextChunkNameoffset = Zlib::ConvertBytesToInt(VREF_Uncompressed, offset + 4, pkg.IsBigEndian);

				NameSize = (NextChunkNameoffset + (81 + (C.ChunkCount * 4))) - (ChunkNameoffset + (81 + (C.ChunkCount * 4)));

				BYTES ChunkName = Walnut::OpenFileDialog::CopyBytes(VREF_Uncompressed, ChunkNameoffset + (81 + (C.ChunkCount * 4)), NameSize);

				offset += 4;

				VREF.ChunkNames.push_back(Zlib::ConvertBytesToString(ChunkName));
				
			}

			offset = 77;

			//NameBlockSize (4 bytes)
			uint32_t NameBlockSize = Zlib::ConvertBytesToInt(VREF_Uncompressed, offset, pkg.IsBigEndian);

			//Begining of InfoBlock
			offset = (NameBlockSize + 81 + (C.ChunkCount * 4)) + 4;
			
			
			for (int i = 0; i < (C.ChunkCount - 1); i++) {
				ChunkInfo chunkinfo;
				ChunkInfoOffsets chunkinfooffsets;
				chunkinfo.ChunkName = VREF.ChunkNames[i];
				//chunk id (4 bytes)
				chunkinfo.ID = Zlib::ConvertBytesToInt(VREF_Uncompressed, offset, pkg.IsBigEndian);
				offset += 4;
				uint32_t VDATOFFSET = offset;
				chunkinfooffsets.VDAT_Offset_Location = offset;
				chunkinfo.VDAT_Offset = Zlib::ConvertBytesToInt(VREF_Uncompressed, offset, pkg.IsBigEndian);
				offset += 4;
				uint32_t VDATSIZE = offset;
				chunkinfooffsets.VDAT_Size_Location = offset;
				chunkinfo.VDAT_Size = Zlib::ConvertBytesToInt(VREF_Uncompressed, offset, pkg.IsBigEndian);
				offset += 4;
				chunkinfo.VDAT_File_Data_1 = VREF_Uncompressed[offset];
				offset += 1;
				chunkinfo.VDAT_File_Data_2 = VREF_Uncompressed[offset];
				offset += 1;
				if (VREF_Uncompressed.size() < (offset + 4)) {
					chunkinfo.OffsetLocations = chunkinfooffsets;
					VREF.ChunkInfos.push_back(chunkinfo);
				}
				else {
					uint32_t NextID = Zlib::ConvertBytesToInt(VREF_Uncompressed, offset, pkg.IsBigEndian);
					if (NextID == chunkinfo.ID) {
						chunkinfo.HasVGPU = true;
						offset += 4;
						uint32_t VGPUOFFSET = offset;
						chunkinfooffsets.VGPU_Offset_Location = offset;
						chunkinfo.VGPU_Offset = Zlib::ConvertBytesToInt(VREF_Uncompressed, offset, pkg.IsBigEndian);
						offset += 4;
						uint32_t VGPUSIZE = offset;
						chunkinfooffsets.VGPU_Size_Location = offset;
						chunkinfo.VGPU_Size = Zlib::ConvertBytesToInt(VREF_Uncompressed, offset, pkg.IsBigEndian);
						offset += 4;
						chunkinfo.VGPU_File_Data_1 = VREF_Uncompressed[offset];
						offset += 1;
						chunkinfo.VGPU_File_Data_2 = VREF_Uncompressed[offset];
						offset += 1;
						chunkinfo.OffsetLocations = chunkinfooffsets;
						VREF.ChunkInfos.push_back(chunkinfo);
					}
					else {
						chunkinfo.HasVGPU = false;
						chunkinfo.OffsetLocations = chunkinfooffsets;
						VREF.ChunkInfos.push_back(chunkinfo);
					}
				}
				
			}
				
			
			pkg.VREFs.push_back(VREF);

			//for each chunk in latest VREF

			//setup timer
			Walnut::Timer timer;
			

			#pragma omp parallel for num_threads(omp_get_max_threads()/2)
			for (int cc = 0; cc < pkg.VREFs[pkg.VREFs.size() - 1].ChunkInfos.size(); cc++) {
			
				
				if (pkg.VREFs[pkg.VREFs.size() - 1].ChunkInfos[cc].ChunkName.find("2.42") != std::string::npos) {
					#pragma omp critical
					{
					BYTES VGPU1 = { 0x00 };
					//FileType RealFileType GetFileType(pkg.VREFs[pkg.VREFs.size() - 1].ChunkInfos[cc].ChunkName, GetChunkVDATBYTES(pkg.VREFs.size() - 1, pkg.VREFs[pkg.VREFs.size() - 1].ChunkInfos[cc].ID - 1, pkg, file), VGPU1)
					
					pkg.VREFs[pkg.VREFs.size() - 1].ChunkInfos[cc].Type = FileType::DDS;
					}
				}
			}
			
			
			//end timer
			std::cout << "Time took: " << timer.ElapsedMillis() << std::endl;

			
		}

		file.close();
		return pkg;
	}
	
	//Works in unreal so all i need to do is port over the code
	inline static Streams UpdateStreams(VREF VREF, ChunkInfo ChunkInfo, BYTES VDAT, BYTES VGPU) {
		return {}; //Need to Port over the code from Unreal to here
	}

	//Replaces a chunk in a PKG file (WIP) - Works in unreal so all i need to do is port over the code
	inline static void ReplaceChunk(PKG pkg, uint32_t CAFFNumber, std::string ChunkName, ChunkType Type, std::string PatchFilePath, std::string NewPKGExportPath) {
		
		//Open Patch File and read data into BYTES
		std::ifstream patchfile(PatchFilePath, std::ios::binary);
		if (!patchfile.is_open()) {
			std::cout << "Failed to open patch file: " << PatchFilePath << std::endl;
			return;
		}
		std::cout << "Reading patch file... " << std::endl;
		Walnut::Timer timer;
		BYTES PatchFileData;
		while (!patchfile.eof()) {
			PatchFileData.push_back(patchfile.get());
		}
		patchfile.close();
		std::cout << "Patch file read in: " << timer.ElapsedMillis() << "ms" << std::endl;
	
		//For each vref in the pkg
		for (int vref_i = 0; vref_i < pkg.VREFs.size(); vref_i++) {
			//For each chunk in the vref
			for (int chunk_i = 0; chunk_i < pkg.VREFs[vref_i].ChunkInfos.size(); chunk_i++) {
				ChunkInfo* IndexedChunk = &pkg.VREFs[vref_i].ChunkInfos[chunk_i];
				if (IndexedChunk->ChunkName == ChunkName) {
					BYTES OGVGPUChunk = GetChunkVGPUBYTES(vref_i, chunk_i, pkg, patchfile);
					BYTES OGVDATChunk = GetChunkVDATBYTES(vref_i, chunk_i, pkg, patchfile);

					Streams NewStreams;

					//Check if the chunk is the same as the patch file and if the chunk is the same size
					switch (Type) {
					case ChunkType::VDAT:
						if (OGVDATChunk.size() == PatchFileData.size()) {
							if (OGVDATChunk == PatchFileData) {
								std::cout << "Chunk is the same!" << std::endl;
								std::cout << "Aborting..." << std::endl;
								return;
							}
							else {
								std::cout << "Patching Data Stream" << std::endl;
								NewStreams = UpdateStreams(pkg.VREFs[vref_i], pkg.VREFs[vref_i].ChunkInfos[chunk_i], PatchFileData, OGVGPUChunk);
							}
						}
						else {
							std::cout << "Chunk is different size!" << std::endl;
							std::cout << "Aborting..." << std::endl;
							return;
						}
						break;
					case ChunkType::VGPU:
						if (OGVGPUChunk.size() == PatchFileData.size()) {
							if (OGVGPUChunk == PatchFileData) {
								std::cout << "Chunk is the same!" << std::endl;
								std::cout << "Aborting..." << std::endl;
								return;
							}
							else {

								std::cout << "Patching GPU Stream" << std::endl;
								NewStreams = UpdateStreams(pkg.VREFs[vref_i], pkg.VREFs[vref_i].ChunkInfos[chunk_i], OGVDATChunk, PatchFileData);

							}
						}
						else {
							std::cout << "Chunk is different size!" << std::endl;
							std::cout << "Aborting..." << std::endl;
							return;
						}
						break;
					}

					std::cout << "Replacing Functionality not implemented yet!" << std::endl;
					return;

					//Update CAFF
					BYTES NewCAFF; //Need to Port over the code from Unreal to here

					//Update CAFF

					//Update PKG with new CAFF
					BYTES NewPKG; //Need to Port over the code from Unreal to here

					//write new pkg to file
					Walnut::OpenFileDialog::AskSaveFile(NewPKG, NewPKGExportPath);

				}
			}
		}
			

	}

}