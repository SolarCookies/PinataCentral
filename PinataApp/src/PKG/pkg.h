#pragma once
#include <stdint.h>
#include <vector>
#include <string>
#include "../Utils/ZLibHelpers.h"
#include "../Utils/OpenFileDialog.h"
#include <iostream>
#include <filesystem>

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

//macro that returns nearest multiple of s from x (Greater then or equal too) | r =(((x - (x % s )) + s) - x) | Maybe ((4014080 / 2048) + 1) * 2048?
//#define NEAREST_MULTIPLE(x,s) (((x - (x % s )) + s) - x);
//#define NEAREST_MULTIPLE(x,s) (((x / s) + 1) * 2048); //get next multiple of 2048
#define NEAREST_MULTIPLE(x,s) (x * 2); //Doubles the offset (technically a multiple of 2 but large)
//#define NEAREST_MULTIPLE(x,s) (x);


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
		Walnut::Timer timer;
		PKG pkg;
		BYTES VDAT;
		BYTES VGPU;
		pkg.path = path;

		std::ifstream file(path, std::ios::binary);
		if (!file.is_open()) {
			Log("Failed to open PKG: " + path, EType::Error);
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
		Log("PKG Version: " + std::to_string(pkg.Version), EType::Normal);
		Log("Is PKG Big Endian: " + std::to_string(pkg.IsBigEndian), EType::Normal);

		//Read CAFF Count (next 4 bytes)
		BYTES CAFFCount = Walnut::OpenFileDialog::Read_Bytes(file, 4);
		pkg.CAFFCount = Zlib::ConvertBytesToInt(CAFFCount, pkg.IsBigEndian);
		Log("CAFF Count: " + std::to_string(pkg.CAFFCount), EType::Normal);

		//Read CAFF Info
		for (int i = 0; i < pkg.CAFFCount; i++) {
			CAFF_Info caff_info;
			//Read CAFF Info
			caff_info.PKGpath = path;
			caff_info.Number = i + 1;
			caff_info.Unknown = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::Read_Bytes(file, 4), pkg.IsBigEndian);
			caff_info.Offset = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::Read_Bytes(file, 4), pkg.IsBigEndian);
			caff_info.Size = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::Read_Bytes(file, 4), pkg.IsBigEndian);
			pkg.CAFF_Infos.push_back(caff_info);
		}

		//Read CAFF Headers
		for (CAFF_Info C : pkg.CAFF_Infos) {

			Log("Reading CAFF: " + std::to_string(C.Number), EType::BLUE);
			//Seek offset
			Walnut::OpenFileDialog::SeekBeg(file, C.Offset);
			CAFF caff;
			caff.CAFF_Info = C;
			//CAFF Version (17 bytes)
			BYTES CAFF_Version_B = Walnut::OpenFileDialog::Read_Bytes(file, 17);
			std::string CAFF_Version = "";
			CAFF_Version = Zlib::ConvertBytesToString(CAFF_Version_B);
			caff.CAFF_Version = CAFF_Version;

			Log("CAFF Version: " + CAFF_Version, EType::Normal);

			//forword 3 bytes of null data
			Walnut::OpenFileDialog::SeekCur(file, 3);

			//VREF Offset (4 bytes)
			caff.VREF_Offset = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::Read_Bytes(file, 4), pkg.IsBigEndian);
			Log("VREF Offset: " + std::to_string(caff.VREF_Offset), EType::Normal);

			//Skip 4 unknown bytes
			Walnut::OpenFileDialog::SeekCur(file, 4);

			//Chunk Amount (4 bytes)
			caff.ChunkCount = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::Read_Bytes(file, 4), pkg.IsBigEndian);

			Log("Chunk Count: " + std::to_string(caff.ChunkCount), EType::Normal);

			//chunk spread count (4 bytes)
			caff.ChunkSpreadCount = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::Read_Bytes(file, 4), pkg.IsBigEndian);

			Log("Chunk Spread Count: " + std::to_string(caff.ChunkSpreadCount), EType::Normal);

			//skip 44 bytes
			Walnut::OpenFileDialog::SeekCur(file, 44);

			//VRef Uncompressed Size (4 bytes)
			caff.VREF_Uncompressed_Size = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::Read_Bytes(file, 4), pkg.IsBigEndian);

			Log("VREF Uncompressed Size: " + std::to_string(caff.VREF_Uncompressed_Size), EType::Normal);

			//skip 12 bytes
			Walnut::OpenFileDialog::SeekCur(file, 12);

			//VRef Compressed Size (4 bytes)
			caff.VREF_Compressed_Size = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::Read_Bytes(file, 4), pkg.IsBigEndian);

			Log("VREF Compressed Size: " + std::to_string(caff.VREF_Compressed_Size), EType::Normal);

			//VLUT Uncompressed Size (4 bytes)
			caff.VLUT_Uncompressed_Size = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::Read_Bytes(file, 4), pkg.IsBigEndian);

			Log("VLUT Uncompressed Size: " + std::to_string(caff.VLUT_Uncompressed_Size), EType::Normal);

			//skip 12 bytes

			Walnut::OpenFileDialog::SeekCur(file, 12);

			//VLUT Compressed Size (4 bytes)
			caff.VLUT_Compressed_Size = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::Read_Bytes(file, 4), pkg.IsBigEndian);
			std::cout << "VLUT Compressed Size: " << caff.VLUT_Compressed_Size << std::endl;

			Log("VLUT Compressed Size: " + std::to_string(caff.VLUT_Compressed_Size), EType::Normal);

			//VLUT Offset is VRef Offset + VRef Compressed Size
			caff.VLUT_Offset = caff.VREF_Offset + caff.VREF_Compressed_Size;
			std::cout << "VLUT Offset: " << caff.VLUT_Offset << std::endl;

			Log("VLUT Offset: " + std::to_string(caff.VLUT_Offset), EType::Normal);

			pkg.CAFFs.push_back(caff);
		}

		//Read VREFs
		for (CAFF C : pkg.CAFFs) {

			Log("Reading VREF: " + std::to_string(C.CAFF_Info.Number), EType::BLUE);

			VREF VREF;

			BYTES VREF_Uncompressed = GetVREFBYTES(file, C);

			//start at first value
			uint32_t offset = 9;

			VREF.VDAT_Uncompressed_Size = Zlib::ConvertBytesToInt(VREF_Uncompressed, offset, pkg.IsBigEndian);
			Log("VDAT Uncompressed Size: " + std::to_string(VREF.VDAT_Uncompressed_Size), EType::Normal);

			offset += 20;

			//VDAT Compressed Size (4 bytes)
			VREF.VDAT_Compressed_Size = Zlib::ConvertBytesToInt(VREF_Uncompressed, offset, pkg.IsBigEndian);
			Log("VDAT Compressed Size: " + std::to_string(VREF.VDAT_Compressed_Size), EType::Normal);


			offset += 20;

			bool VREFType = VREFTypeCheck(VREF_Uncompressed);

			offset = 42;

			//VGPU Uncompressed Size (4 bytes)
			VREF.VGPU_Uncompressed_Size = Zlib::ConvertBytesToInt(VREF_Uncompressed, offset, pkg.IsBigEndian);
			Log("VGPU Uncompressed Size: " + std::to_string(VREF.VGPU_Uncompressed_Size), EType::Normal);

			offset += 20;

			//VGPU Compressed Size (4 bytes)
			VREF.VGPU_Compressed_Size = Zlib::ConvertBytesToInt(VREF_Uncompressed, offset, pkg.IsBigEndian);
			Log("VGPU Compressed Size: " + std::to_string(VREF.VGPU_Compressed_Size), EType::Normal);

			offset = 81; //? WTH why am i setting the offset to 81? if im not even using it? -_-

			//VLUT_OFFSET + VLUT_Compressed_Size
			VREF.VDAT_Offset = C.VLUT_Offset + C.VLUT_Compressed_Size;
			Log("VDAT Offset: " + std::to_string(VREF.VDAT_Offset), EType::Normal);

			//VDAT_OFFSET + VDAT_Compressed_Size
			VREF.VGPU_Offset = VREF.VDAT_Offset + VREF.VDAT_Compressed_Size;
			Log("VGPU Offset: " + std::to_string(VREF.VGPU_Offset), EType::Normal);

			Log("Reading Chunk Info...", EType::BLUE);
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
			Log("VREF Read", EType::GREEN);


			pkg.VREFs.push_back(VREF);

			//for each chunk in latest VREF

			//setup timer
			Walnut::Timer Typestimer;

			Log("Checking Chunk Types...", EType::BLUE);

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
			Log("Chunk Types Checked", EType::GREEN);
			Log("Time took reading chunk types: " + std::to_string(Typestimer.ElapsedMillis()), EType::PURPLE);


		}

		file.close();
		Log("PKG Read", EType::GREEN);
		Log("Time took reading PKG: " + std::to_string(timer.ElapsedMillis()), EType::PURPLE);

		return pkg;
	}

	//Returns full vref, vdat, vgpu streams patched with new data
	inline static Streams UpdateStreams(PKG pkg, VREF VREF, CAFF Caff, ChunkInfo ChunkInfo, BYTES VDAT, BYTES VGPU, bool DDS = false) {
		Streams PatchedStreams;

		std::ifstream file(pkg.path, std::ios::binary);
		if (!file.is_open()) {
			std::cout << "Failed to open file: " << pkg.path << std::endl;
			return PatchedStreams;
		}

		
		Log("Reading VREF");
		PatchedStreams.VREF = GetVREFBYTES(file, Caff);

		Log("Reading VDAT on CAFF: " + std::to_string(Caff.CAFF_Info.Number - 1), EType::Normal);
		PatchedStreams.VDAT = GetVDATBYTES(Caff.CAFF_Info.Number - 1, pkg, file);

		Log("Reading VGPU on CAFF: " + std::to_string(Caff.CAFF_Info.Number - 1), EType::Normal);
		PatchedStreams.VGPU = GetVGPUBYTES(Caff.CAFF_Info.Number - 1, pkg, file);

		file.close();

		Log("Patching VDAT at: " + std::to_string(ChunkInfo.VDAT_Offset) + " with size: " + std::to_string(ChunkInfo.VDAT_Size), EType::Normal);

		
		//Patching VDAT
		for (int i = 0; i < VDAT.size(); i++) {
			PatchedStreams.VDAT[ChunkInfo.VDAT_Offset + i] = VDAT[i];
		}

		
		if (ChunkInfo.HasVGPU){
			Log("Patching VGPU at: " + std::to_string(ChunkInfo.VGPU_Offset) + " with size: " + std::to_string(ChunkInfo.VGPU_Size), EType::Normal);

			//Append bytes at the beginning of the VGPU with ".dds" if DDS is true
			if (DDS) {
				VGPU.insert(VGPU.begin(), { 0x2E, 0x64, 0x64, 0x64 });
				//resize VGPU to ChunkInfo.Size
				PatchedStreams.VGPU.resize(ChunkInfo.VGPU_Size);
			}

			//Patching VGPU
			for (int i = 0; i < VGPU.size(); i++) {
				PatchedStreams.VGPU[ChunkInfo.VGPU_Offset + i] = VGPU[i];
			}

			
			Log("Patched VGPU", EType::Normal);
		}

		Log("Patched Streams..", EType::Normal);


		return PatchedStreams; // BP code:(https://blueprintue.com/blueprint/nzpxzrdk/)
	}

	//Returns full vref, vdat, vgpu streams patched with new data
	inline static Streams UpdateStreams_WithDoubleOffsetSpacing(PKG pkg, VREF VREF, CAFF Caff, ChunkInfo ChunkInfo, BYTES VDAT, BYTES VGPU) {
		Streams PatchedStreams;

		std::ifstream filer(pkg.path, std::ios::binary);
		if (!filer.is_open()) {
			std::cout << "Failed to open file: " << pkg.path << std::endl;
			return PatchedStreams;
		}


		Log("Reading VREF");
		PatchedStreams.VREF = GetVREFBYTES(filer, Caff);

		Log("Reading VDAT on CAFF: " + std::to_string(Caff.CAFF_Info.Number - 1), EType::Normal);
		PatchedStreams.VDAT = GetVDATBYTES(Caff.CAFF_Info.Number - 1, pkg, filer);

		Log("Reading VGPU on CAFF: " + std::to_string(Caff.CAFF_Info.Number - 1), EType::Normal);
		PatchedStreams.VGPU = GetVGPUBYTES(Caff.CAFF_Info.Number - 1, pkg, filer);

		filer.close();

		Log("Creating New VDAT and VGPU with double Offset spacing", EType::Warning);

		BYTES OGVGPU = PatchedStreams.VGPU;
		BYTES OGVDAT = PatchedStreams.VDAT;

		for (int i = 0; i < VREF.ChunkInfos.size(); i++) {
			//resize VDAT to ChunkInfo.Offset * 2
			PatchedStreams.VDAT.resize(VREF.ChunkInfos[i].VDAT_Offset * 2);

			BYTES VDATChunk;

			if (VREF.ChunkInfos[i].ChunkName == ChunkInfo.ChunkName) {
				//Patch VDAT with new VDAT
				VDATChunk = VDAT;
			}
			else {
				//Copy bytes from old VDAT to new VDAT
				VDATChunk = Walnut::OpenFileDialog::CopyBytes(OGVDAT, VREF.ChunkInfos[i].VDAT_Offset, VREF.ChunkInfos[i].VDAT_Size);
			}
			
			//Copy VDATChunk to new VDAT
			PatchedStreams.VDAT.insert(PatchedStreams.VDAT.begin() + (VREF.ChunkInfos[i].VDAT_Offset * 2) , VDATChunk.begin(), VDATChunk.end());
			//Patching VDAT


			//ChunkInfo.OffsetLocations.VDAT_Offset_Location needs to be updated to new offset in the vref
			Walnut::OpenFileDialog::SetIntAtOffset(PatchedStreams.VREF, VREF.ChunkInfos[i].OffsetLocations.VDAT_Offset_Location, ChunkInfo.VDAT_Offset * 2, pkg.IsBigEndian);

			

			if (VREF.ChunkInfos[i].HasVGPU) {
				//resize VGPU to ChunkInfo.Offset * 2 + ChunkInfo.Size
				PatchedStreams.VGPU.resize(VREF.ChunkInfos[i].VGPU_Offset * 2);

				BYTES VGPUChunk;
				if (VREF.ChunkInfos[i].ChunkName == ChunkInfo.ChunkName) {
					//Patch VGPU with new VGPU
					VGPUChunk = VGPU;
				}
				else {
					//Copy bytes from old VGPU to new VGPU
					VGPUChunk = Walnut::OpenFileDialog::CopyBytes(OGVGPU, VREF.ChunkInfos[i].VGPU_Offset, VREF.ChunkInfos[i].VGPU_Size);
				}
				//Copy VGPUChunk to new VGPU
				PatchedStreams.VGPU.insert(PatchedStreams.VGPU.begin() + (VREF.ChunkInfos[i].VGPU_Offset * 2), VGPUChunk.begin(), VGPUChunk.end());
				//Patching VGPU
				//ChunkInfo.OffsetLocations.VGPU_Offset_Location needs to be updated to new offset in the vref
				Walnut::OpenFileDialog::SetIntAtOffset(PatchedStreams.VREF, VREF.ChunkInfos[i].OffsetLocations.VGPU_Offset_Location, ChunkInfo.VGPU_Offset * 2, pkg.IsBigEndian);
			}
			
		}

		//Technically we now have the new VDAT and VGPU with double offset spacing and the vref has been updated with the new offsets


		return PatchedStreams; // BP code:(https://blueprintue.com/blueprint/nzpxzrdk/)
	}

	//Returns full vref, vdat, vgpu streams patched with new data
	inline static Streams UpdateStreams_WithOneOffsetSpacing(PKG pkg, VREF VREF, CAFF Caff, ChunkInfo ChunkInfo, BYTES VDAT, BYTES VGPU) {
		Streams PatchedStreams;

		std::ifstream filer(pkg.path, std::ios::binary);
		if (!filer.is_open()) {
			std::cout << "Failed to open file: " << pkg.path << std::endl;
			return PatchedStreams;
		}


		Log("Reading VREF");
		PatchedStreams.VREF = GetVREFBYTES(filer, Caff);

		Log("Reading VDAT on CAFF: " + std::to_string(Caff.CAFF_Info.Number - 1), EType::Normal);
		PatchedStreams.VDAT = GetVDATBYTES(Caff.CAFF_Info.Number - 1, pkg, filer);

		Log("Reading VGPU on CAFF: " + std::to_string(Caff.CAFF_Info.Number - 1), EType::Normal);
		PatchedStreams.VGPU = GetVGPUBYTES(Caff.CAFF_Info.Number - 1, pkg, filer);

		filer.close();

		Log("Creating New VDAT and VGPU with new Offset spacing", EType::Warning);

		BYTES OGVGPU = PatchedStreams.VGPU;
		BYTES OGVDAT = PatchedStreams.VDAT;

		for (int i = 0; i < VREF.ChunkInfos.size(); i++) {

			BYTES VDATChunk;

			if (VREF.ChunkInfos[i].ChunkName == ChunkInfo.ChunkName) {
				//Patch VDAT with new VDAT
				VDATChunk = VDAT;
				uint32_t NewVDAT_Offset = PatchedStreams.VDAT.size();
				PatchedStreams.VDAT.insert(PatchedStreams.VDAT.begin() + NewVDAT_Offset, VDATChunk.begin(), VDATChunk.end());
				Walnut::OpenFileDialog::SetIntAtOffset(PatchedStreams.VREF, VREF.ChunkInfos[i].OffsetLocations.VDAT_Offset_Location, NewVDAT_Offset, pkg.IsBigEndian);
			}
			else {
				//Copy bytes from old VDAT to new VDAT
				VDATChunk = Walnut::OpenFileDialog::CopyBytes(OGVDAT, VREF.ChunkInfos[i].VDAT_Offset, VREF.ChunkInfos[i].VDAT_Size);
				uint32_t NewVDAT_Offset = PatchedStreams.VDAT.size();
				PatchedStreams.VDAT.insert(PatchedStreams.VDAT.begin() + NewVDAT_Offset, VDATChunk.begin(), VDATChunk.end());
				Walnut::OpenFileDialog::SetIntAtOffset(PatchedStreams.VREF, VREF.ChunkInfos[i].OffsetLocations.VDAT_Offset_Location, NewVDAT_Offset, pkg.IsBigEndian);
			}


			if (VREF.ChunkInfos[i].HasVGPU) {

				BYTES VGPUChunk;
				if (VREF.ChunkInfos[i].ChunkName == ChunkInfo.ChunkName) {
					//Patch VGPU with new VGPU
					VGPUChunk = VGPU;
					uint32_t NewVGPU_Offset = PatchedStreams.VGPU.size();
					PatchedStreams.VGPU.insert(PatchedStreams.VGPU.begin() + NewVGPU_Offset, VGPUChunk.begin(), VGPUChunk.end());
					Walnut::OpenFileDialog::SetIntAtOffset(PatchedStreams.VREF, VREF.ChunkInfos[i].OffsetLocations.VDAT_Offset_Location, NewVGPU_Offset, pkg.IsBigEndian);
				}
				else {
					//Copy bytes from old VGPU to new VGPU
					VGPUChunk = Walnut::OpenFileDialog::CopyBytes(OGVGPU, VREF.ChunkInfos[i].VGPU_Offset, VREF.ChunkInfos[i].VGPU_Size);
					uint32_t NewVGPU_Offset = PatchedStreams.VGPU.size();
					PatchedStreams.VGPU.insert(PatchedStreams.VGPU.begin() + NewVGPU_Offset, VGPUChunk.begin(), VGPUChunk.end());
					Walnut::OpenFileDialog::SetIntAtOffset(PatchedStreams.VREF, VREF.ChunkInfos[i].OffsetLocations.VDAT_Offset_Location, NewVGPU_Offset, pkg.IsBigEndian);
				}
			}

		}


		return PatchedStreams; 
	}

	//Returns full vref, vdat, vgpu streams patched with new data
	inline static Streams UpdateStreams_AppendEnd(PKG pkg, VREF VREF, CAFF Caff, ChunkInfo ChunkInfo, BYTES VDAT, BYTES VGPU) {
		Streams PatchedStreams;

		std::ifstream file(pkg.path, std::ios::binary);
		if (!file.is_open()) {
			std::cout << "Failed to open file: " << pkg.path << std::endl;
			return PatchedStreams;
		}


		Log("Reading VREF");
		PatchedStreams.VREF = GetVREFBYTES(file, Caff);

		Log("Reading VDAT on CAFF: " + std::to_string(Caff.CAFF_Info.Number - 1), EType::Normal);
		PatchedStreams.VDAT = GetVDATBYTES(Caff.CAFF_Info.Number - 1, pkg, file);

		Log("Reading VGPU on CAFF: " + std::to_string(Caff.CAFF_Info.Number - 1), EType::Normal);
		PatchedStreams.VGPU = GetVGPUBYTES(Caff.CAFF_Info.Number - 1, pkg, file);

		file.close();

		Log("Patching VDAT at: " + std::to_string(ChunkInfo.VDAT_Offset) + " with size: " + std::to_string(ChunkInfo.VDAT_Size), EType::Normal);


		//Patching VDAT
		for (int i = 0; i < VDAT.size(); i++) {
			PatchedStreams.VDAT[ChunkInfo.VDAT_Offset + i] = VDAT[i];
		}


		if (ChunkInfo.HasVGPU) {
			Log("Patching VGPU at: " + std::to_string(ChunkInfo.VGPU_Offset) + " with size: " + std::to_string(ChunkInfo.VGPU_Size), EType::Normal);

			
			//Patching VGPU
			//for (int i = 0; i < VGPU.size(); i++) {
			//	PatchedStreams.VGPU[ChunkInfo.VGPU_Offset + i] = VGPU[i];
			//}
			
			//resize VGPU to have a small buffer at the end
			PatchedStreams.VGPU.resize(PatchedStreams.VGPU.size() + 0x10);
			uint32_t NewVGPU_Offset = PatchedStreams.VGPU.size();
			PatchedStreams.VGPU.insert(PatchedStreams.VGPU.begin() + NewVGPU_Offset, VGPU.begin(), VGPU.end());
			//Update VREF with new VGPU offset
			Walnut::OpenFileDialog::SetIntAtOffset(PatchedStreams.VREF, ChunkInfo.OffsetLocations.VGPU_Offset_Location, NewVGPU_Offset, pkg.IsBigEndian);


			Log("Patched VGPU", EType::Normal);
		}

		Log("Patched Streams..", EType::Normal);


		return PatchedStreams; // BP code:(https://blueprintue.com/blueprint/nzpxzrdk/)
	}

	//Returns full caff patched with new data 
	inline static BYTES UpdateCAFF(PKG pkg, VREF VREF, CAFF Caff, Streams NewStreams, bool BigEndian) {

		BYTES NewCAFF;

		std::ifstream file(pkg.path, std::ios::binary);
		if (!file.is_open()) {
			Log("Failed to open file: " + VREF.CAFF.PKGpath, EType::Error);
			return NewCAFF;
		}

		int VREF_Uncompressed_Size = NewStreams.VREF.size();
		Log("NewStreams.VREF Uncompressed Size: " + std::to_string(VREF_Uncompressed_Size), EType::Normal);

		int VDAT_Uncompressed_Size = NewStreams.VDAT.size();
		Log("NewStreams.VDAT Uncompressed Size: " + std::to_string(VDAT_Uncompressed_Size), EType::Normal);

		int VGPU_Uncompressed_Size = NewStreams.VGPU.size();
		Log("NewStreams.VGPU Uncompressed Size: " + std::to_string(VGPU_Uncompressed_Size), EType::Normal);

		//Compress VDAT
		BYTES VDAT_Compressed = Zlib::CompressData(NewStreams.VDAT);
		Log("New VDAT Compressed Size: " + std::to_string(VDAT_Compressed.size()), EType::Normal);

		//Compress VGPU
		BYTES VGPU_Compressed = Zlib::CompressData(NewStreams.VGPU);
		Log("New VGPU Compressed Size: " + std::to_string(VGPU_Compressed.size()), EType::Normal);

		BYTES ModifiedVRef = NewStreams.VREF;

		//Set VREF VDAT Uncompressed Size at offset 9
		Log("Setting VDAT Uncompressed Size at offset 9: " + std::to_string(VDAT_Uncompressed_Size), EType::Normal);
		Walnut::OpenFileDialog::SetIntAtOffset(ModifiedVRef, 9, VDAT_Uncompressed_Size, BigEndian);

		//Set VREF VDAT Compressed Size at offset 29
		Log("Setting VDAT Compressed Size at offset 29: " + std::to_string(VDAT_Compressed.size()), EType::Normal);
		Walnut::OpenFileDialog::SetIntAtOffset(ModifiedVRef, 29, VDAT_Compressed.size(), BigEndian);

		//Set VREF VGPU Uncompressed Size at offset 42
		Log("Setting VGPU Uncompressed Size at offset 42: " + std::to_string(VGPU_Uncompressed_Size), EType::Normal);
		Walnut::OpenFileDialog::SetIntAtOffset(ModifiedVRef, 42, VGPU_Uncompressed_Size, BigEndian);

		//Set VREF VGPU Compressed Size at offset 62
		Log("Setting VGPU Compressed Size at offset 62: " + std::to_string(VGPU_Compressed.size()), EType::Normal);
		Walnut::OpenFileDialog::SetIntAtOffset(ModifiedVRef, 62, VGPU_Compressed.size(), BigEndian);


		Log("Old VREF Size: " + std::to_string(NewStreams.VREF.size()) + " New VREF Size: " + std::to_string(ModifiedVRef.size()), EType::Normal);

		
		//compress VREF
		BYTES VREF_Compressed = Zlib::CompressData(ModifiedVRef);
		
		Log("Old VREF Compressed Size: " + std::to_string(Caff.VREF_Compressed_Size) + " New VREF Compressed Size: " + std::to_string(VREF_Compressed.size()), EType::Normal);
		Log("VREF Compressed Size: " + std::to_string(VREF_Compressed.size()), EType::Normal);	

		BYTES VLUT_Uncompressed = Zlib::DecompressData(Walnut::OpenFileDialog::Read_Bytes(file, Caff.VLUT_Offset + Caff.CAFF_Info.Offset, Caff.VLUT_Compressed_Size), Caff.VLUT_Uncompressed_Size);


		BYTES VLUT_Compressed = Zlib::CompressData(VLUT_Uncompressed);

		Log("VLUT Compressed Size: " + std::to_string(VLUT_Compressed.size()), EType::Normal);

		BYTES CAFFHeader = Walnut::OpenFileDialog::Read_Bytes(file, Caff.CAFF_Info.Offset, Caff.VREF_Offset);

		//Set VREF Uncompressed Size at offset 80 of CAFFHeader
		Walnut::OpenFileDialog::SetIntAtOffset(CAFFHeader, 80, VREF_Uncompressed_Size, BigEndian);

		//Set VREF Compressed Size at offset 96 of CAFFHeader
		Walnut::OpenFileDialog::SetIntAtOffset(CAFFHeader, 96, VREF_Compressed.size(), BigEndian);

		//Calculate new CAFF Checksum

		//0 out old checksum
		for (int i = 0x18; i < 0x1C; i++)
		{
			CAFFHeader[i] = 0;
		}

		//Calculate new checksum using MOJOBOJO's algorithm
		uint32_t NewChecksum = Zlib::CAFF_checksum(CAFFHeader);

		//Set new checksum at offset 24
		Walnut::OpenFileDialog::SetIntAtOffset(CAFFHeader, 0x18, NewChecksum, BigEndian);


		//construct new CAFF
		NewCAFF = CAFFHeader;
		NewCAFF.insert(NewCAFF.end(), VREF_Compressed.begin(), VREF_Compressed.end());
		NewCAFF.insert(NewCAFF.end(), VLUT_Compressed.begin(), VLUT_Compressed.end());
		NewCAFF.insert(NewCAFF.end(), VDAT_Compressed.begin(), VDAT_Compressed.end());
		NewCAFF.insert(NewCAFF.end(), VGPU_Compressed.begin(), VGPU_Compressed.end());

		return NewCAFF; // BP code:(https://blueprintue.com/blueprint/vv97_nrk/)
	}

	//Returns full pkg patched with new data
	inline static BYTES UpdatePKG(PKG pkg, BYTES NewCAFF, int CAFFNumber) {
		//Open PKG 
		std::ifstream file(pkg.path, std::ios::binary);

		BYTES PKG = Walnut::OpenFileDialog::Read_Bytes(file, 0, pkg.CAFF_Infos[CAFFNumber].Offset);
		BYTES NewPKG;

		std::vector<CAFF_Info> NewCAFFInfos;

		//For each CAFF Info
		for (int i = 0; i < pkg.CAFF_Infos.size(); i++) {

			Log("Working on CAFF Number: " + std::to_string(i) + " Out of: " + std::to_string(pkg.CAFF_Infos.size()), EType::Normal);

			CAFF_Info NewCAFFInfo;

			int CaffOffset = pkg.CAFF_Infos[i].Offset;
			int CaffSize = pkg.CAFF_Infos[i].Size;
			int CaffNumber = pkg.CAFF_Infos[i].Number;
			int CaffUnknown = pkg.CAFF_Infos[i].Unknown;

			NewCAFFInfo.Number = CaffNumber;
			NewCAFFInfo.Unknown = CaffUnknown;

			if (i == 0) {
				//Copy PKG Header + Empty Space until first CAFF offset
				NewPKG.insert(NewPKG.end(), PKG.begin(), PKG.begin() + pkg.CAFF_Infos[0].Offset);
			}

			
			if (CaffNumber - 1 == CAFFNumber) {
				//Set New CAFF Infos

				//int NextOffset = NEAREST_MULTIPLE(CaffOffset, 2048);
				//std::cout << "Old CAFF Offset: " << CaffOffset << " New CAFF Offset: " << NextOffset << std::endl;

				int NextOffset = NEAREST_MULTIPLE(CaffOffset, 2048); //Figure out a way to get the next 2048 offset evenually

				Log("Old CAFF Offset: " + std::to_string(CaffOffset) + " New CAFF Offset: " + std::to_string(NextOffset), EType::Normal);

				//resize NewPKG to fit new CAFF offset (if needed)
				if (NextOffset > NewPKG.size()) {
					Log("Resizing NewPKG to fit new CAFF offset", EType::Normal);
					NewPKG.resize(NextOffset);
					Log("NewPKG Size: " + std::to_string(NewPKG.size()), EType::Normal);
				}

				NewCAFFInfo.Offset = NextOffset;

				//Insert New CAFF
				NewPKG.insert(NewPKG.end(), NewCAFF.begin(), NewCAFF.end());

				NewCAFFInfo.Size = NewCAFF.size();

				NewCAFFInfos.push_back(NewCAFFInfo);
			}
			else {

				//int NextOffset = NEAREST_MULTIPLE(CaffOffset, 2048);
				
				int NextOffset = NEAREST_MULTIPLE(CaffOffset, 2048); //Figure out a way to get the next 2048 offset evenually

				Log("Old CAFF Offset: " + std::to_string(CaffOffset) + " New CAFF Offset: " + std::to_string(NextOffset), EType::Normal);

				//resize NewPKG to fit new CAFF offset (if needed)
				if (NextOffset > NewPKG.size()) {
					Log("Resizing NewPKG to fit new CAFF offset", EType::Normal);
					NewPKG.resize(NextOffset);
					Log("NewPKG Size: " + std::to_string(NewPKG.size()), EType::Normal);
				}

				NewCAFFInfo.Offset = NextOffset;

				//Copy CAFF
				BYTES CAFF = Walnut::OpenFileDialog::Read_Bytes(file, CaffOffset, CaffSize);
				NewPKG.insert(NewPKG.end(), CAFF.begin(), CAFF.end());

				NewCAFFInfo.Size = CAFF.size();

				NewCAFFInfos.push_back(NewCAFFInfo);
			}
		}

		//Set New PKG CAFF Infos
		for (int i = 0; i < NewCAFFInfos.size(); i++) {
			
			//set CAFF unknown at offset ((Caff Number - 1) * 12) + 8
			int UnknownOffset = ((NewCAFFInfos[i].Number - 1) * 12) + 8;
			Log("Old Unknown: " + std::to_string(pkg.CAFF_Infos[i].Unknown) + " New Unknown: " + std::to_string(NewCAFFInfos[i].Unknown), EType::Normal);

			
			Log("Writing Unknown at offset: " + std::to_string(UnknownOffset), EType::Normal);
			Walnut::OpenFileDialog::SetIntAtOffset(NewPKG, UnknownOffset, NewCAFFInfos[i].Unknown, pkg.IsBigEndian);

			//set CAFF offset at offset (((Caff Number - 1) * 12) + 8) + 4
			int OffsetOffset = ((NewCAFFInfos[i].Number - 1) * 12) + 8 + 4;
			Log("Writing CAFF Offset at offset: " + std::to_string(OffsetOffset), EType::Normal);

			Log("Writing Offset at offset: " + std::to_string(OffsetOffset), EType::Normal);
			Walnut::OpenFileDialog::SetIntAtOffset(NewPKG, OffsetOffset, NewCAFFInfos[i].Offset, pkg.IsBigEndian);

			//set CAFF size at offset (((Caff Number - 1) * 12) + 8) + 8
			int SizeOffset = ((NewCAFFInfos[i].Number - 1) * 12) + 8 + 8;
			Log("Old Size: " + std::to_string(pkg.CAFF_Infos[i].Size) + " New Size: " + std::to_string(NewCAFFInfos[i].Size), EType::Normal);
			
			Log("Writing Size at offset: " + std::to_string(SizeOffset), EType::Normal);
			Walnut::OpenFileDialog::SetIntAtOffset(NewPKG, SizeOffset, NewCAFFInfos[i].Size, pkg.IsBigEndian);
		}


		return { NewPKG }; // BP code:(https://blueprintue.com/blueprint/6dd8sfb8/
	}

	//Replaces a chunk in a PKG file (WIP) - Works in unreal so all i need to do is port over the code
	inline static void ReplaceChunk(PKG pkg, uint32_t CAFFNumber, std::string ChunkName, ChunkType Type, std::string PatchFilePath, std::string NewPKGExportPath, bool OveridePKG, bool DDS = false) {

		//Open Patch File and read data into BYTES
		std::ifstream patchfile(PatchFilePath, std::ios::binary);
		if (!patchfile.is_open()) {
			Log("Failed to open patch file: " + PatchFilePath, EType::Error);
			return;
		}
		Log("Reading patch file...", EType::Normal);
		Walnut::Timer timer;
		BYTES PatchFileData;
		while (!patchfile.eof()) {
			PatchFileData.push_back(patchfile.get());
		}
		patchfile.close();


		std::ifstream file(pkg.path, std::ios::binary);

		//remove last byte of patch file data, Removes a null byte at the end of the file that is added by the while loop?
		PatchFileData.pop_back();

		Log("Patch file read in: " + std::to_string(timer.ElapsedMillis()) + "ms", EType::PURPLE);

		//For each vref in the pkg
		for (int vref_i = 0; vref_i < pkg.VREFs.size(); vref_i++) {
			//For each chunk in the vref
			for (int chunk_i = 0; chunk_i < pkg.VREFs[vref_i].ChunkInfos.size(); chunk_i++) {
				
				ChunkInfo* IndexedChunk = &pkg.VREFs[vref_i].ChunkInfos[chunk_i];
				if (IndexedChunk->ChunkName == ChunkName) {

					Log("Working on VREF: " + std::to_string(vref_i) + " Out of: " + std::to_string(pkg.VREFs.size()), EType::Normal);
					Log("Working on Chunk: " + std::to_string(chunk_i) + " Out of: " + std::to_string(pkg.VREFs[vref_i].ChunkInfos.size()), EType::Normal);

					BYTES OGVGPUChunk;
					if (IndexedChunk->HasVGPU) {
						OGVGPUChunk = GetChunkVGPUBYTES(vref_i, chunk_i, pkg, file);
					}


					BYTES OGVDATChunk = GetChunkVDATBYTES(vref_i, chunk_i, pkg, file);
					

					Streams NewStreams;

					//Check if the chunk is the same as the patch file and if the chunk is the same size
					switch (Type) {
					case ChunkType::VDAT:
						if (OGVDATChunk.size() == PatchFileData.size()) {
							if (OGVDATChunk == PatchFileData) {
								Log("Chunk is the same!, Aborting...", EType::Error);
								return;
							}
							else {
								Log("Patching Data Stream", EType::Normal);
								NewStreams = UpdateStreams_AppendEnd(pkg, pkg.VREFs[vref_i], pkg.CAFFs[vref_i], pkg.VREFs[vref_i].ChunkInfos[chunk_i], PatchFileData, OGVGPUChunk);
							}
						}
						else {
							Log("Chunk is different size!, Aborting...", EType::Error);
							Log("OGVDATChunk Size: " + std::to_string(OGVDATChunk.size()), EType::Error);
							Log("PatchFileData Size: " + std::to_string(PatchFileData.size()), EType::Error);
							return;
						}
						break;
					case ChunkType::VGPU:
						Log("Patching GPU Stream", EType::Normal);
						NewStreams = UpdateStreams_AppendEnd(pkg, pkg.VREFs[vref_i], pkg.CAFFs[vref_i], pkg.VREFs[vref_i].ChunkInfos[chunk_i], OGVDATChunk, PatchFileData);
						break;
					}

					

					//Update CAFF
					BYTES NewCAFF = UpdateCAFF(pkg, pkg.VREFs[vref_i], pkg.CAFFs[vref_i], NewStreams, pkg.IsBigEndian);

					//Update PKG with new CAFF
					BYTES NewPKG = UpdatePKG(pkg, NewCAFF, vref_i);

					Log("Writing new PKG file...", EType::Normal);
					
					file.close();

					if (OveridePKG) {
						//check to see if a backup of the original PKG exists at /backup/.pkg

						std::string BackupPath = "Backups\\PackageBundles\\" + pkg.path.substr(pkg.path.find_last_of("\\") + 1);
						std::filesystem::create_directories("Backups\\PackageBundles\\");
						std::filesystem::path BACKUPPATH = BackupPath;
						std::filesystem::path PKGPATH = pkg.path;


						//copy file to backup if it doesn't exist
						if (!std::filesystem::exists(BACKUPPATH)) {
							Log("Creating backup of original PKG file at: " + BackupPath, EType::Normal);
							std::filesystem::copy_file(PKGPATH, BACKUPPATH);
						}else{
							Log("Backup of original PKG file already exists at: " + BackupPath, EType::Normal);
							Log("Skiping backup...", EType::Normal);
						}

						//replace original PKG with new PKG
						std::filesystem::remove(PKGPATH);
						std::ofstream newpkgfile(pkg.path, std::ios::binary);
						if (!newpkgfile.is_open()) {
							Log("Failed to open new PKG file: " + pkg.path, EType::Error);
						}
						else {
							newpkgfile.write((char*)NewPKG.data(), NewPKG.size());
							newpkgfile.close();
							Log("Patched PKG File", EType::GREEN);
							return;
						}


					}
					else {

						//Write new PKG to file
						std::string NewPKGExportPath = NewPKGExportPath + "\\Patched_" + pkg.path.substr(pkg.path.find_last_of("\\") + 1);
						std::ofstream newpkgfile(NewPKGExportPath, std::ios::binary);
						if (!newpkgfile.is_open()) {
							Log("Failed to open new PKG file: " + NewPKGExportPath, EType::Error);
						}
						else {
							newpkgfile.write((char*)NewPKG.data(), NewPKG.size());
							newpkgfile.close();
							Log("New PKG file written!", EType::GREEN);
						}
					}
					

				}
			}
		}


	}

	//Replaces a chunk in a PKG file (WIP) - Works in unreal so all i need to do is port over the code
	inline static void ReplaceChunkWithBytes(PKG pkg, uint32_t CAFFNumber, std::string ChunkName, ChunkType Type, BYTES PatchFileData) {

		std::ifstream file(pkg.path, std::ios::binary);

		//For each vref in the pkg
		for (int vref_i = 0; vref_i < pkg.VREFs.size(); vref_i++) {
			//For each chunk in the vref
			for (int chunk_i = 0; chunk_i < pkg.VREFs[vref_i].ChunkInfos.size(); chunk_i++) {

				ChunkInfo* IndexedChunk = &pkg.VREFs[vref_i].ChunkInfos[chunk_i];
				if (IndexedChunk->ChunkName == ChunkName) {

					Log("Working on VREF: " + std::to_string(vref_i) + " Out of: " + std::to_string(pkg.VREFs.size()), EType::Normal);
					Log("Working on Chunk: " + std::to_string(chunk_i) + " Out of: " + std::to_string(pkg.VREFs[vref_i].ChunkInfos.size()), EType::Normal);

					BYTES OGVGPUChunk;
					if (IndexedChunk->HasVGPU) {
						OGVGPUChunk = GetChunkVGPUBYTES(vref_i, chunk_i, pkg, file);
					}


					BYTES OGVDATChunk = GetChunkVDATBYTES(vref_i, chunk_i, pkg, file);


					Streams NewStreams;

					//Check if the chunk is the same as the patch file and if the chunk is the same size
					switch (Type) {
					case ChunkType::VDAT:
						if (OGVDATChunk.size() == PatchFileData.size()) {
							if (OGVDATChunk == PatchFileData) {
								Log("Chunk is the same!, Aborting...", EType::Error);
								return;
							}
							else {
								Log("Patching Data Stream", EType::Normal);
								NewStreams = UpdateStreams_AppendEnd(pkg, pkg.VREFs[vref_i], pkg.CAFFs[vref_i], pkg.VREFs[vref_i].ChunkInfos[chunk_i], PatchFileData, OGVGPUChunk);
							}
						}
						else {
							Log("Chunk is different size!, Aborting...", EType::Error);
							Log("OGVDATChunk Size: " + std::to_string(OGVDATChunk.size()), EType::Error);
							Log("PatchFileData Size: " + std::to_string(PatchFileData.size()), EType::Error);
							return;
						}
						break;
					case ChunkType::VGPU:
						Log("Patching GPU Stream", EType::Normal);
						NewStreams = UpdateStreams_AppendEnd(pkg, pkg.VREFs[vref_i], pkg.CAFFs[vref_i], pkg.VREFs[vref_i].ChunkInfos[chunk_i], OGVDATChunk, PatchFileData);
						break;
					}



					//Update CAFF
					BYTES NewCAFF = UpdateCAFF(pkg, pkg.VREFs[vref_i], pkg.CAFFs[vref_i], NewStreams, pkg.IsBigEndian);

					//Update PKG with new CAFF
					BYTES NewPKG = UpdatePKG(pkg, NewCAFF, vref_i);

					Log("Writing new PKG file...", EType::Normal);

					file.close();

					
					//check to see if a backup of the original PKG exists at /backup/.pkg

					std::string BackupPath = "Backups\\PackageBundles\\" + pkg.path.substr(pkg.path.find_last_of("\\") + 1);
					std::filesystem::create_directories("Backups\\PackageBundles\\");
					std::filesystem::path BACKUPPATH = BackupPath;
					std::filesystem::path PKGPATH = pkg.path;


					//copy file to backup if it doesn't exist
					if (!std::filesystem::exists(BACKUPPATH)) {
						Log("Creating backup of original PKG file at: " + BackupPath, EType::Normal);
						std::filesystem::copy_file(PKGPATH, BACKUPPATH);
					}
					else {
						Log("Backup of original PKG file already exists at: " + BackupPath, EType::Normal);
						Log("Skiping backup...", EType::Normal);
					}

					//replace original PKG with new PKG
					std::filesystem::remove(PKGPATH);
					std::ofstream newpkgfile(pkg.path, std::ios::binary);
					if (!newpkgfile.is_open()) {
						Log("Failed to open new PKG file: " + pkg.path, EType::Error);
					}
					else {
						newpkgfile.write((char*)NewPKG.data(), NewPKG.size());
						newpkgfile.close();
						Log("Patched PKG File", EType::GREEN);
						return;
					}


					
					


				}
			}
		}


	}

}
