#pragma once
#include "../../../Utils/ZLibHelpers.h"


struct Aid_Loctext_Header {
	int offset;
	int unk1;
	int unk2;
};

struct Aid_Loctext_LSBL {
	char MAGIC[4];
	int Unk1;
	int Unk2;
	int Unk3;
	int TagsTableOffset;
	int CommentsLabelTableOffset;
	int TableOffset;
};

struct Aid_Loctext_StringTableHeader {
	int TotalSectionLength;
	int TotalStrings;
};

#pragma pack(push, 1)
struct Aid_Loctext_StringInfo {
	uint16_t ID;
	uint32_t Offset;
};
#pragma pack(pop)

struct Aid_Loctext_TagsTableHeader {
	int TotalSectionLength;
	int TotalTags;
};

struct Aid_Loctext_TagsInfo {
	uint16_t ID;
	uint16_t unk;
	int TagStringOffset;
};

class Aid_Loctext
{
	public:
		vBYTES FILE;
		Aid_Loctext_Header header;
		Aid_Loctext_LSBL lsbl;
		Aid_Loctext_StringTableHeader stringTableHeader;
		Aid_Loctext_TagsTableHeader tagsTable;

		std::vector<std::string> Strings;
		std::vector<std::string> Tags;

		Aid_Loctext() = default;

		Aid_Loctext(vBYTES File) {
			FILE = File;

			memcpy(&header, FILE.data(), sizeof(Aid_Loctext_Header));
			memcpy(&lsbl, FILE.data() + header.offset, sizeof(Aid_Loctext_LSBL));
			memcpy(&stringTableHeader, FILE.data() + header.offset + sizeof(Aid_Loctext_LSBL), sizeof(Aid_Loctext_StringTableHeader));
			memcpy(&tagsTable, FILE.data() + lsbl.TagsTableOffset + header.offset, sizeof(Aid_Loctext_TagsTableHeader));

			stringTableHeader.TotalStrings++;

			for(int i = 0; i < stringTableHeader.TotalStrings; i++) {
				std::cout << "String Info " << i << std::endl;
				std::cout << stringTableHeader.TotalStrings << std::endl;
				Aid_Loctext_StringInfo info;
				memcpy(&info, FILE.data() + header.offset + sizeof(Aid_Loctext_LSBL) + sizeof(Aid_Loctext_StringTableHeader) + (sizeof(Aid_Loctext_StringInfo) * i), sizeof(Aid_Loctext_StringInfo));

				std::string str;

				int offset = (info.Offset*2) + header.offset + sizeof(Aid_Loctext_LSBL) + sizeof(Aid_Loctext_StringTableHeader) + (sizeof(Aid_Loctext_StringInfo) * stringTableHeader.TotalStrings);
				if (stringTableHeader.TotalStrings - 1 == i) {
					//int length = (lsbl.TagsTableOffset + header.offset) - offset;
					//std::cout << "Last --- Length: " << length << " NextOffset: " << (lsbl.TagsTableOffset + header.offset) << " Offset: " << offset << std::endl;
					continue;
				}
				else {
					
					Aid_Loctext_StringInfo nextinfo;
					memcpy(&nextinfo, FILE.data() + header.offset + sizeof(Aid_Loctext_LSBL) + sizeof(Aid_Loctext_StringTableHeader) + (sizeof(Aid_Loctext_StringInfo) * (i + 1)), sizeof(Aid_Loctext_StringInfo));
					int nextoffset = (nextinfo.Offset * 2) + header.offset + sizeof(Aid_Loctext_LSBL) + sizeof(Aid_Loctext_StringTableHeader) + (sizeof(Aid_Loctext_StringInfo) * stringTableHeader.TotalStrings);
					int length = nextoffset - offset;
					std::cout << "Normal --- Length: " << length << " NextOffset: " << nextoffset << " Offset: " << offset << std::endl;
					
					while(FILE[offset] != 0x00) {
						str += FILE[offset];
						offset += 2;
					}
				}
				
				Strings.push_back(str);
			}

			for(int i = 0; i < tagsTable.TotalTags; i++) {
				std::cout << "Tag Info " << i << std::endl;
				Aid_Loctext_TagsInfo info;
				memcpy(&info, FILE.data() + lsbl.TagsTableOffset + header.offset + sizeof(Aid_Loctext_TagsTableHeader) + (sizeof(Aid_Loctext_TagsInfo) * i), sizeof(Aid_Loctext_TagsInfo));
				std::string tag;
				int offset = info.TagStringOffset + header.offset + lsbl.TagsTableOffset + sizeof(Aid_Loctext_TagsTableHeader) + (sizeof(Aid_Loctext_TagsInfo) * tagsTable.TotalTags);
				while(FILE[offset] != 0x00) {
					tag += FILE[offset];
					offset += 1;
				}
				Tags.push_back(tag);
			}
        
		}
		~Aid_Loctext() = default;

		std::string GetTextByTag(std::string tag)
		{
			for(int i = 0; i < Tags.size(); i++) {
				if (Tags[i] == tag) {
					return Strings[i];
				}
			}
		}
};