#pragma once
#include <vector>
#include <string>
#include "../Utils/ZLibHelpers.h"
#include "../Utils/OpenFileDialog.h"

//Offset to this can be found at offset 0
struct ModelHeader {
	int Type;
	int RendergraphOffset;
};

struct Rendergraph {
	int modelTableOffset;
	int unk1; //Offset
	int unk2;
	int unk3; //Offset
	int textureNameTableOffset;
	int textureMapTableOffset;
	int NumberOfTextures;
	int unk5;
	int morphTargetTableOffset;
	int morphTargetTableCount;
	int inputDefinitionTableOffset;
	int unk6;
	int unk7;
	int unk8; //Offset
	int unk9;
	int unk10;
	int unk11; //Offset to some kind of table that points to shadow map names
	int unk12;
	int unk13;
	int unk14; //Offset
	int unk15;
	int unk16; //Offset
};

static inline Rendergraph GetRendergraph(BYTES VDAT) {
	BYTES HeaderOffsetBytes = { VDAT[0], VDAT[1], VDAT[2], VDAT[3] };
	int ModelHeaderOffset = Zlib::ConvertBytesToInt(HeaderOffsetBytes, 0, false);
	BYTES RendergraphOffsetBytes = { VDAT[ModelHeaderOffset + 4], VDAT[ModelHeaderOffset + 5], VDAT[ModelHeaderOffset + 6], VDAT[ModelHeaderOffset + 7] };
	int RendergraphOffset = Zlib::ConvertBytesToInt(RendergraphOffsetBytes, 0, false);
	Rendergraph RG;
	memcpy(&RG, &VDAT[RendergraphOffset], sizeof(Rendergraph));
	return RG;
}

static inline std::vector<std::string> GetModelTextureNames(BYTES VDAT) {
	Rendergraph RG = GetRendergraph(VDAT);
	std::vector<std::string> TextureNames;
	if (RG.NumberOfTextures == 0 || RG.textureNameTableOffset == 0) return TextureNames; //No textures
	for (int i = 0; i < RG.NumberOfTextures; i++) {
		int TextureNameOffset = RG.textureNameTableOffset + (i * 4);
		BYTES TextureNameOffsetBytes = { VDAT[TextureNameOffset], VDAT[TextureNameOffset + 1], VDAT[TextureNameOffset + 2], VDAT[TextureNameOffset + 3] };
		int ActualTextureNameOffset = Zlib::ConvertBytesToInt(TextureNameOffsetBytes, 0, false);
		if (ActualTextureNameOffset == 0) continue; //Skip empty textures (They do appear in some models)
		std::string TextureName = "";
		for (int j = ActualTextureNameOffset; VDAT[j] != 0; j++) {
			TextureName += VDAT[j];
		}
		TextureNames.push_back(TextureName);
	}
	return TextureNames;
}

static inline std::vector<std::string> GetModelTextureMapNames(BYTES VDAT) {
	Rendergraph RG = GetRendergraph(VDAT);
	std::vector<std::string> TextureMapNames;
	if (RG.textureMapTableOffset == 0) return TextureMapNames; //No texture maps
	for (int i = 0; i < RG.NumberOfTextures; i++) {
		int TextureMapNameOffset = RG.textureMapTableOffset + (i * 4);
		BYTES TextureMapNameOffsetBytes = { VDAT[TextureMapNameOffset], VDAT[TextureMapNameOffset + 1], VDAT[TextureMapNameOffset + 2], VDAT[TextureMapNameOffset + 3] };
		int ActualTextureMapNameOffset = Zlib::ConvertBytesToInt(TextureMapNameOffsetBytes, 0, false);
		if (ActualTextureMapNameOffset == 0) continue; //Skip empty textures (They do appear in some models)
		std::string TextureMapName = "";
		for (int j = ActualTextureMapNameOffset; VDAT[j] != 0; j++) {
			TextureMapName += VDAT[j];
		}
		TextureMapNames.push_back(TextureMapName);
	}
	return TextureMapNames;
}