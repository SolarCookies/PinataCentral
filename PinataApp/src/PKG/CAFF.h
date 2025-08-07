#pragma once
#include <string>
#include <iostream>
#include <filesystem>
#include "../Utils/ZLibHelpers.h"
#include "../Utils/OpenFIleDialog.h"

enum class FileType
{
	Unknown,
	DDS,
	RawImage,
	XUI_Scene,
	Model
};

enum class ChunkType
{
	Unknown,
	VDAT,
	VGPU,
	DDS,
	Model
};

struct CAFF_Info {
	std::string PKGpath = "";
	uint32_t Offset = 0;
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
	bool IsBigEndian = false;
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
	BYTE VDAT_File_Data_1 = 0;
	BYTE VDAT_File_Data_2 = 0;
	bool HasVGPU = false;
	uint32_t VGPU_Offset = 0;
	uint32_t VGPU_Size = 0;
	BYTE VGPU_File_Data_1 = 0;
	BYTE VGPU_File_Data_2 = 0;
	ChunkInfoOffsets OffsetLocations;
	FileType Type = FileType::Unknown;
	BYTE DebugData = 0; //Used for debugging when checking if a offset is correct
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
	uint32_t NameBlockOffset = 0;
	uint32_t InfoBlockOffset = 0;
	std::vector<ChunkInfo> ChunkInfos;
};

namespace caff {

	//Reads CAFF Header when given CAFF bytes (At least the first 120 bytes as that is the header) but you can pass in the full CAFF
	CAFF Read_Header(BYTES CAFFBYTES) {
		CAFF caff;
		caff.CAFF_Version = Zlib::ConvertBytesToString(Walnut::OpenFileDialog::CopyBytes(CAFFBYTES, 0, 17));

		//The vref offset/caff header size is always 120 so we can use that to our advantage to get the endianess
		if (Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::CopyBytes(CAFFBYTES, 20, 4), true) == 120) {
			caff.IsBigEndian = true;
		}
		else {
			caff.IsBigEndian = false;
		}

		caff.VREF_Offset = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::CopyBytes(CAFFBYTES, 20, 4), caff.IsBigEndian);
		caff.ChunkCount = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::CopyBytes(CAFFBYTES, 28, 4), caff.IsBigEndian);
		caff.ChunkSpreadCount = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::CopyBytes(CAFFBYTES, 32, 4), caff.IsBigEndian);
		caff.VREF_Uncompressed_Size = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::CopyBytes(CAFFBYTES, 80, 4), caff.IsBigEndian);
		caff.VREF_Compressed_Size = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::CopyBytes(CAFFBYTES, 96, 4), caff.IsBigEndian);
		caff.VLUT_Uncompressed_Size = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::CopyBytes(CAFFBYTES, 100, 4), caff.IsBigEndian);
		caff.VLUT_Compressed_Size = Zlib::ConvertBytesToInt(Walnut::OpenFileDialog::CopyBytes(CAFFBYTES, 116, 4), caff.IsBigEndian);

		caff.VLUT_Offset = caff.VREF_Offset + caff.VREF_Compressed_Size;
		return caff;
	}

	//Gets the VREF Bytes from the CAFF (Works with compressed and uncompressed CAFFs)
	BYTES Get_VREF(BYTES& CAFFBYTES, CAFF& caff) {

		//Some CAFFs are uncompressed, when this is the case the Compressed size is = to the uncompressed size
		if (caff.VREF_Compressed_Size == 0 || caff.VREF_Compressed_Size == caff.VREF_Uncompressed_Size) {
			BYTES VREF = Walnut::OpenFileDialog::CopyBytes(CAFFBYTES, caff.VREF_Offset, caff.VREF_Uncompressed_Size);
			return VREF;
		}

		//The caff seems to be compressed, so we need to decompress it using zlib
		BYTES VREFCompressed = Walnut::OpenFileDialog::CopyBytes(CAFFBYTES, caff.VREF_Offset, caff.VREF_Compressed_Size);
		BYTES VREF = Zlib::DecompressData(VREFCompressed, caff.VREF_Uncompressed_Size);
		return VREF;
	}

	bool VREFTypeCheck(BYTES& VREF) {
		bool isDataOnly = false;

		//Data would be "1635017060" in little endian because it is "data" in ASCII
		uint32_t DataType = Zlib::ConvertBytesToInt(VREF, 34, false);
		
		if (DataType == 1635017060) {
			isDataOnly = true;
			return isDataOnly;
		}

		isDataOnly = false;
		return isDataOnly;
	}

	//Reads the VREF chunk info and fills the VREF struct with the chunk names and chunk info
	void Read_VREF_Chunks(BYTES& VREFBYTES, CAFF& caff, VREF& vref, bool BigEndian) {

		uint32_t offset;

		if (VREFTypeCheck(VREFBYTES)) {
			offset = 43; //43 is the start of the chunk name offset (Not the name itself but the offset to the name)
		}
		else {
			offset = 81; //81 is the start of the chunk name offset (Not the name itself but the offset to the name)
		}

		

		//Forward declare the chunk name offset and size so we are not allocating memory in the loop
		uint32_t ChunkNameoffset;
		uint32_t NextChunkNameoffset;
		uint32_t NameSize;
		BYTES ChunkName;

		//for each chunk we get the name by getting the offset to the name and then getting the name from the name block
		for (int i = 0; i <= (caff.ChunkCount - 1); i++) {

			//If this is the last chunk, we need to use the NameBlock size rather then the next chunk name offset
			if (i == (caff.ChunkCount - 1)) {
				ChunkNameoffset = Zlib::ConvertBytesToInt(VREFBYTES, offset, BigEndian);

				//NameSize = the size of the name block - the size of the chunk name offset
				NameSize = vref.InfoBlockOffset - (vref.NameBlockOffset + ChunkNameoffset);

				ChunkName = Walnut::OpenFileDialog::CopyBytes(VREFBYTES, vref.NameBlockOffset + ChunkNameoffset, NameSize);

				offset += 4;

				vref.ChunkNames.push_back(Zlib::ConvertBytesToString(ChunkName));

				continue;
			}

			ChunkNameoffset = Zlib::ConvertBytesToInt(VREFBYTES, offset, BigEndian);

			NextChunkNameoffset = Zlib::ConvertBytesToInt(VREFBYTES, offset + 4, BigEndian);

			NameSize = NextChunkNameoffset - ChunkNameoffset;

			ChunkName = Walnut::OpenFileDialog::CopyBytes(VREFBYTES, vref.NameBlockOffset + ChunkNameoffset, NameSize);

			offset += 4;

			vref.ChunkNames.push_back(Zlib::ConvertBytesToString(ChunkName));
		}

		//Info Block starts after a buffer of 4 bytes in-between the name block and the info block
		offset = vref.InfoBlockOffset + 4;

		//For each chunk we get the chunk info from the info block
		for (int i = 0; i <= (caff.ChunkCount - 1); i++) {
			ChunkInfo chunkinfo;
			ChunkInfoOffsets chunkinfooffsets;
			chunkinfo.ChunkName = vref.ChunkNames[i];
			//chunk id (4 bytes)
			chunkinfo.ID = Zlib::ConvertBytesToInt(VREFBYTES, offset, BigEndian);
			offset += 4;
			uint32_t VDATOFFSET = offset;
			chunkinfooffsets.VDAT_Offset_Location = offset;
			chunkinfo.VDAT_Offset = Zlib::ConvertBytesToInt(VREFBYTES, offset, BigEndian);
			offset += 4;
			uint32_t VDATSIZE = offset;
			chunkinfooffsets.VDAT_Size_Location = offset;
			chunkinfo.VDAT_Size = Zlib::ConvertBytesToInt(VREFBYTES, offset, BigEndian);
			offset += 4;
			chunkinfo.VDAT_File_Data_1 = VREFBYTES[offset];
			offset += 1;
			chunkinfo.VDAT_File_Data_2 = VREFBYTES[offset];
			offset += 1;
			if (VREFBYTES.size() < (offset + 4)) {
				chunkinfo.OffsetLocations = chunkinfooffsets;
				vref.ChunkInfos.push_back(chunkinfo);
			}
			else {
				uint32_t NextID = Zlib::ConvertBytesToInt(VREFBYTES, offset, BigEndian);
				if (NextID == chunkinfo.ID) {
					chunkinfo.HasVGPU = true;
					offset += 4;
					uint32_t VGPUOFFSET = offset;
					chunkinfooffsets.VGPU_Offset_Location = offset;
					chunkinfo.VGPU_Offset = Zlib::ConvertBytesToInt(VREFBYTES, offset, BigEndian);
					offset += 4;
					uint32_t VGPUSIZE = offset;
					chunkinfooffsets.VGPU_Size_Location = offset;
					chunkinfo.VGPU_Size = Zlib::ConvertBytesToInt(VREFBYTES, offset, BigEndian);
					offset += 4;
					chunkinfo.VGPU_File_Data_1 = VREFBYTES[offset];
					offset += 1;
					chunkinfo.VGPU_File_Data_2 = VREFBYTES[offset];
					offset += 1;
					chunkinfo.OffsetLocations = chunkinfooffsets;
					vref.ChunkInfos.push_back(chunkinfo);
				}
				else {
					chunkinfo.HasVGPU = false;
					chunkinfo.OffsetLocations = chunkinfooffsets;
					vref.ChunkInfos.push_back(chunkinfo);
				}
			}
		}


	}

	//Reads the VREF Header and Chunk infos
	VREF Read_VREF(BYTES& VREFBYTES, CAFF& caff, bool BigEndian) {
		VREF vref;

		vref.VDAT_Uncompressed_Size = Zlib::ConvertBytesToInt(VREFBYTES, 9, BigEndian);
		vref.VDAT_Compressed_Size = Zlib::ConvertBytesToInt(VREFBYTES, 29, BigEndian);

		if (VREFTypeCheck(VREFBYTES)) {
			vref.VGPU_Uncompressed_Size = 0;
			vref.VGPU_Compressed_Size = 0;
			vref.NameBlockOffset = 43 + (caff.ChunkCount * 4);
			vref.InfoBlockOffset = Zlib::ConvertBytesToInt(VREFBYTES, 39, BigEndian) + vref.NameBlockOffset;
		}
		else {
			vref.VGPU_Uncompressed_Size = Zlib::ConvertBytesToInt(VREFBYTES, 42, BigEndian);
			vref.VGPU_Compressed_Size = Zlib::ConvertBytesToInt(VREFBYTES, 62, BigEndian);
			vref.NameBlockOffset = 81 + (caff.ChunkCount * 4);
			vref.InfoBlockOffset = Zlib::ConvertBytesToInt(VREFBYTES, 77, BigEndian) + vref.NameBlockOffset;
		}
		
		vref.VDAT_Offset = caff.VLUT_Offset + caff.VLUT_Compressed_Size;
		vref.VGPU_Offset = vref.VDAT_Offset + vref.VDAT_Compressed_Size;

		Read_VREF_Chunks(VREFBYTES, caff, vref, BigEndian);

		return vref;
	}

}
