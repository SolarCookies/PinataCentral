#pragma once
#include <vector>
#include <string>
#include "../Utils/ZLibHelpers.h"
#include "../Utils/OpenFileDialog.h"
#include "../Utils/Log.hpp"
#include "../Utils/half.hpp"
#include "VertexBlockTypes.h"




struct Vertex1 {
    Vector3 position; // 12 bytes
    Vector3 normal; // 12 bytes
    Vector2 texCoord; // 8 bytes
};

//Located at 0x0 in vdat
struct ModelHeader {
    uint32_t FooterOffset;
    uint32_t NumOfEntrys;
    uint32_t unk1;
    uint32_t unk2;
    uint32_t unk3;
};

struct EntryType {
    uint32_t ID;
    uint32_t Offset;
};

struct ModelFooter {
    std::vector<EntryType> Entrys;
};

struct Rendergraph {
    uint32_t ModelInfoOffset;
    uint32_t Unk1;
    uint32_t Unk2;
    uint32_t Unk3;
    uint32_t TextureNamesOffset;
    uint32_t TextureMapsOffset;
    uint32_t NumberOfTextures;
    uint32_t unk5;
    uint32_t morphTargetTableOffset;
    uint32_t morphTargetTableCount;
    uint32_t inputDefinition1TableOffset;
    uint32_t inputDefinition1TableNumber;
    uint32_t inputDefinitionSize;
    uint32_t inputDefinition2TableOffset;
    uint32_t inputDefinition2TableNumber;
    uint32_t unk10;
    uint32_t shadowMapNames;
    uint32_t NumberOfshadowMaps;
    uint32_t unk13;
    uint32_t unk14; //Offset
    uint32_t unk14Count;
    uint32_t unk16; //Offset
    uint32_t unk16Count;
    uint32_t unk17;
    uint32_t unk18;
    uint32_t unk19;
    uint32_t unk20;
    uint32_t unk21;
    uint32_t unk22;
    uint32_t unk23;
    uint32_t unk24;
    uint32_t unk25;
    uint32_t unk26;
    uint32_t unk27;
    uint32_t unk28;
    uint32_t unk29;
    float unk30;
    float unk31;
    uint32_t unk32;
};

struct ModelInfo {
    uint8_t Type;
    uint8_t Flag2;
    uint8_t Flag3;
    uint8_t ID;
    uint32_t NextOffset;
    uint32_t AdditonalOffset;
    uint32_t LastOffset;
};

struct ModelVertDef
{
    ModelInfo info;
    uint16_t unk1;
    uint16_t unk2;
    uint32_t unk3;
    uint32_t vertexCount;
    uint32_t dataOffset;
    uint32_t vertexOffset;
    uint32_t vertTableLength;
    uint16_t entrySize;
    uint16_t unk5;
};

struct ModelIndicesDef
{
    ModelInfo info;
    int unk1;

    // Not entirely sure about this, as some models load the indices fine and others don't.
    int IndicesSize;
    int IndicesCount;
    int unk4;
    int IndicesCount2;
    int unk6;
    int IndicesOffset;
    char unk7[28];
    int IndicesOffset2;
    int IndicesOffset3;
};

struct ModelBlock {
    ModelVertDef VertDef;
    ModelIndicesDef IndiceDef;
};

inline static int visitedOffsets[1024];
inline static int visitedCount = 0;
inline static int offsetsToVisit[1024];
inline static int offsetCount = 0;

namespace model {



    inline static void exportOBJ(std::vector<Vertex1> verts, std::vector<uint32_t> indices, std::string& filename)
    {
        std::ofstream file(filename);
        if (!file) {
            throw std::runtime_error("Failed to open file for writing");
        }

        // Write vertex positions
        for (const auto& v : verts) {
            file << "v " << v.position.x << " " << v.position.y << " " << v.position.z << "\n";
        }

        // Write vertex normals
        for (const auto& v : verts) {
            file << "vn " << v.normal.x << " " << v.normal.y << " " << v.normal.z << "\n";
        }
        // Write texture coordinates
        for (const auto& v : verts) {
            file << "vt " << v.texCoord.u << " " << v.texCoord.v << "\n";
        }

        // Write faces (OBJ is 1-based indexing)
        for (size_t i = 0; i < indices.size(); i += 3) {
            if (i + 2 < indices.size()) {
                uint32_t i1 = indices[i] + 1;
                uint32_t i2 = indices[i + 1] + 1;
                uint32_t i3 = indices[i + 2] + 1;

                // Position/UV/Normal — assuming they share the same index
                file << "f "
                    << i1 << "/" << i1 << "/" << i1 << " "
                    << i2 << "/" << i2 << "/" << i2 << " "
                    << i3 << "/" << i3 << "/" << i3 << "\n";
            }
        }
        file.close();
    }

   
    inline static ModelVertDef GetNextVertexBlock(ModelVertDef& Current, std::vector<ModelVertDef> Verts)
    {
        int CurrentOffset = Current.vertexOffset;
        ModelVertDef Next;
        Next.vertexOffset = MAXINT; //Initialize to MaxInt so we can use < next comparison
        for (ModelVertDef& vert : Verts) {
            if (vert.vertexOffset == 0) continue; //Next vertex block cant be the first one
            if (vert.vertexOffset > CurrentOffset && vert.vertexOffset < Next.vertexOffset) {
                Next = vert;
            }
        }
        return Next;
    }

    inline int AlreadyVisited(int offset) {
        for (int i = 0; i < visitedCount; i++) {
            if (visitedOffsets[i] == offset) {
                return 1;
            }
        }
        return 0;
    }

    inline void MarkVisited(int offset) {
        visitedOffsets[visitedCount++] = offset;
    }

    inline void AddOffsetToVisit(int offset) {
        if (offset != 0 && !AlreadyVisited(offset)) {
            offsetsToVisit[offsetCount++] = offset;
            MarkVisited(offset);
        }
    }

    inline vBYTES ReplaceVertsInVGPU(vBYTES& VGPU, ModelVertDef& VertBlock, std::vector<Vector3>& newVerts) {
        vBYTES newVGPU = VGPU; // Start with a copy of the original VGPU data
        // Ensure we don't exceed the original vertex count
        size_t count = std::min(newVerts.size(), static_cast<size_t>(VertBlock.vertexCount));
        for (size_t i = 0; i < count; i++) {
            size_t offset = VertBlock.vertexOffset + i * VertBlock.entrySize;
            if (offset + sizeof(Vector3) <= newVGPU.size()) { // Ensure we don't write out of bounds
                // Directly copy the new vertex data (little-endian)
                std::memcpy(&newVGPU[offset], &newVerts[i], sizeof(Vector3));
            }
            else {
                throw std::runtime_error("Attempting to write out of bounds in VGPU data");
            }
        }
        return newVGPU;
    }

    inline inline Rendergraph GetRendergraph(vBYTES VDAT, bool IsBigEndian) {
        ModelHeader Header;
        memcpy(&Header, &VDAT[0], sizeof(ModelHeader));
        if (IsBigEndian) {
            Header.FooterOffset = _byteswap_ulong(Header.FooterOffset);
            Header.NumOfEntrys = _byteswap_ulong(Header.NumOfEntrys);
            Header.unk1 = _byteswap_ulong(Header.unk1);
            Header.unk2 = _byteswap_ulong(Header.unk2);
            Header.unk3 = _byteswap_ulong(Header.unk3);
        }

        ModelFooter Footer;
        Footer.Entrys.resize(Header.NumOfEntrys);

        for (int i = 0; i < Header.NumOfEntrys; i++) {
            memcpy(&Footer.Entrys[i], &VDAT[Header.FooterOffset + (i * sizeof(EntryType))], sizeof(EntryType));
            if (IsBigEndian) {
                Footer.Entrys[i].ID = _byteswap_ulong(Footer.Entrys[i].ID);
                Footer.Entrys[i].Offset = _byteswap_ulong(Footer.Entrys[i].Offset);
            }
        }

        Rendergraph RG;
        for (const auto& entry : Footer.Entrys) {
            if (entry.ID == 0) { //Rendergraph
                memcpy(&RG, &VDAT[entry.Offset], sizeof(Rendergraph));
                if (IsBigEndian) {
                    RG.ModelInfoOffset = _byteswap_ulong(RG.ModelInfoOffset);
                    RG.Unk1 = _byteswap_ulong(RG.Unk1);
                    RG.Unk2 = _byteswap_ulong(RG.Unk2);
                    RG.Unk3 = _byteswap_ulong(RG.Unk3);
                    RG.TextureNamesOffset = _byteswap_ulong(RG.TextureNamesOffset);
                    RG.TextureMapsOffset = _byteswap_ulong(RG.TextureMapsOffset);
                    RG.NumberOfTextures = _byteswap_ulong(RG.NumberOfTextures);
                    RG.unk5 = _byteswap_ulong(RG.unk5);
                    RG.morphTargetTableOffset = _byteswap_ulong(RG.morphTargetTableOffset);
                    RG.morphTargetTableCount = _byteswap_ulong(RG.morphTargetTableCount);
                    RG.inputDefinition1TableOffset = _byteswap_ulong(RG.inputDefinition1TableOffset);
                    RG.inputDefinition1TableNumber = _byteswap_ulong(RG.inputDefinition1TableNumber);
                    RG.inputDefinitionSize = _byteswap_ulong(RG.inputDefinitionSize);
                    RG.inputDefinition2TableOffset = _byteswap_ulong(RG.inputDefinition2TableOffset);
                    RG.inputDefinition2TableNumber = _byteswap_ulong(RG.inputDefinition2TableNumber);
                    RG.unk10 = _byteswap_ulong(RG.unk10);
                    RG.shadowMapNames = _byteswap_ulong(RG.shadowMapNames);
                    RG.NumberOfshadowMaps = _byteswap_ulong(RG.NumberOfshadowMaps);
                    RG.unk13 = _byteswap_ulong(RG.unk13);
                    RG.unk14 = _byteswap_ulong(RG.unk14);
                    RG.unk14Count = _byteswap_ulong(RG.unk14Count);
                    RG.unk16 = _byteswap_ulong(RG.unk16);
                    RG.unk16Count = _byteswap_ulong(RG.unk16Count);
                    RG.unk17 = _byteswap_ulong(RG.unk17);
                    RG.unk18 = _byteswap_ulong(RG.unk18);
                    RG.unk19 = _byteswap_ulong(RG.unk19);
                    RG.unk20 = _byteswap_ulong(RG.unk20);
                    RG.unk21 = _byteswap_ulong(RG.unk21);
                    RG.unk22 = _byteswap_ulong(RG.unk22);
                    RG.unk23 = _byteswap_ulong(RG.unk23);
                    RG.unk24 = _byteswap_ulong(RG.unk24);
                    RG.unk25 = _byteswap_ulong(RG.unk25);
                    RG.unk26 = _byteswap_ulong(RG.unk26);
                    RG.unk27 = _byteswap_ulong(RG.unk27);
                    RG.unk28 = _byteswap_ulong(RG.unk28);
                    RG.unk29 = _byteswap_ulong(RG.unk29);
                    RG.unk30 = byteswap_float(RG.unk30);
                    RG.unk31 = byteswap_float(RG.unk31);
                    RG.unk32 = _byteswap_ulong(RG.unk32);
                }

                break;
            }
        }
        return RG;

    }

    inline static void ExportModel(vBYTES VDAT, vBYTES VGPU, std::string Path, bool BigEndian)
    {
        Rendergraph RG = GetRendergraph(VDAT, BigEndian);

        AddOffsetToVisit(RG.ModelInfoOffset);

        int currentOffset;
        int LocalOffset;

        std::vector<ModelVertDef> verts;
        std::vector<ModelIndicesDef> indices;

        //Main Model Traversal Loop
        while (offsetCount > 0) {
            currentOffset = offsetsToVisit[--offsetCount];

            ModelInfo mi;
            memcpy(&mi, &VDAT[currentOffset], sizeof(ModelInfo));

            if (BigEndian) {
                mi.NextOffset = _byteswap_ulong(mi.NextOffset);
                mi.AdditonalOffset = _byteswap_ulong(mi.AdditonalOffset);
                mi.LastOffset = _byteswap_ulong(mi.LastOffset);
            }

            LocalOffset = currentOffset + 16;
            if (mi.Type == 2) {
                ModelVertDef vert;
                memcpy(&vert, &VDAT[currentOffset], sizeof(ModelVertDef));
                if (BigEndian) {
                    vert.info.NextOffset = _byteswap_ulong(vert.info.NextOffset);
                    vert.info.AdditonalOffset = _byteswap_ulong(vert.info.AdditonalOffset);
                    vert.info.LastOffset = _byteswap_ulong(vert.info.LastOffset);
                    vert.vertexCount = _byteswap_ulong(vert.vertexCount);
                    vert.dataOffset = _byteswap_ulong(vert.dataOffset);
                    vert.vertexOffset = _byteswap_ulong(vert.vertexOffset);
                    vert.vertTableLength = _byteswap_ulong(vert.vertTableLength);
                    vert.entrySize = _byteswap_ushort(vert.entrySize);
                }
                verts.push_back(vert);
            }
            else if (mi.Type == 6 || mi.Type == 7) {
                ModelIndicesDef indice;
                memcpy(&indice, &VDAT[currentOffset], sizeof(ModelIndicesDef));
                if (BigEndian) {
                    indice.info.NextOffset = _byteswap_ulong(indice.info.NextOffset);
                    indice.info.AdditonalOffset = _byteswap_ulong(indice.info.AdditonalOffset);
                    indice.info.LastOffset = _byteswap_ulong(indice.info.LastOffset);
                    indice.IndicesSize = _byteswap_ulong(indice.IndicesSize);
                    indice.IndicesCount = _byteswap_ulong(indice.IndicesCount);
                    indice.IndicesCount2 = _byteswap_ulong(indice.IndicesCount2);
                    indice.IndicesOffset = _byteswap_ulong(indice.IndicesOffset);
                    indice.IndicesOffset2 = _byteswap_ulong(indice.IndicesOffset2);
                    indice.IndicesOffset3 = _byteswap_ulong(indice.IndicesOffset3);
                }
                indices.push_back(indice);
            }

            // Queue both Next and Additional offsets
            AddOffsetToVisit(mi.NextOffset);
            AddOffsetToVisit(mi.AdditonalOffset);
        }

        std::string outLog = "Found " + std::to_string(verts.size()) + " vertex definitions and " + std::to_string(indices.size()) + " indices definitions in model\n";
        Log(outLog, EType::GREEN);

        //for each vert definition, export the vertices
        int y = 0;
        for (const auto& vert : verts) {
            ModelBlock mb;
            mb.VertDef = vert;

            ModelInfo mi1; //unk
            memcpy(&mi1, &VDAT[vert.info.NextOffset], sizeof(ModelInfo));

            if (BigEndian) {
                mi1.NextOffset = _byteswap_ulong(mi1.NextOffset);
                mi1.AdditonalOffset = _byteswap_ulong(mi1.AdditonalOffset);
                mi1.LastOffset = _byteswap_ulong(mi1.LastOffset);
            }

            ModelInfo mi2; //unk
            memcpy(&mi2, &VDAT[mi1.NextOffset], sizeof(ModelInfo));

            if (BigEndian) {
                mi2.NextOffset = _byteswap_ulong(mi2.NextOffset);
                mi2.AdditonalOffset = _byteswap_ulong(mi2.AdditonalOffset);
                mi2.LastOffset = _byteswap_ulong(mi2.LastOffset);
            }

            ModelIndicesDef mi3; //Always indiceinfo
            memcpy(&mi3, &VDAT[mi2.NextOffset], sizeof(ModelIndicesDef));
            mb.IndiceDef = mi3;

            if (BigEndian) {
                mb.IndiceDef.info.NextOffset = _byteswap_ulong(mb.IndiceDef.info.NextOffset);
                mb.IndiceDef.info.AdditonalOffset = _byteswap_ulong(mb.IndiceDef.info.AdditonalOffset);
                mb.IndiceDef.info.LastOffset = _byteswap_ulong(mb.IndiceDef.info.LastOffset);
                mb.IndiceDef.IndicesSize = _byteswap_ulong(mb.IndiceDef.IndicesSize);
                mb.IndiceDef.IndicesCount = _byteswap_ulong(mb.IndiceDef.IndicesCount);
                mb.IndiceDef.IndicesCount2 = _byteswap_ulong(mb.IndiceDef.IndicesCount2);
                mb.IndiceDef.IndicesOffset = _byteswap_ulong(mb.IndiceDef.IndicesOffset);
                mb.IndiceDef.IndicesOffset2 = _byteswap_ulong(mb.IndiceDef.IndicesOffset2);
                mb.IndiceDef.IndicesOffset3 = _byteswap_ulong(mb.IndiceDef.IndicesOffset3);
            }

            std::vector<Vertex1> vertices1;
            vertices1.resize(mb.VertDef.vertexCount);


            vBYTES vertexData;
            vertexData.resize(mb.VertDef.vertexCount * mb.VertDef.entrySize);
            memcpy(vertexData.data(), &VGPU[mb.VertDef.vertexOffset], mb.VertDef.vertexCount * mb.VertDef.entrySize);



            for (uint32_t i = 0; i < mb.VertDef.vertexCount; i++) {
                Vertex1 v;
                
                vBYTES VertBlockData;
				VertBlockData.resize(mb.VertDef.entrySize);
				
				memcpy(VertBlockData.data(), &vertexData[i * mb.VertDef.entrySize], mb.VertDef.entrySize);

				VertexBlock block = ConstructVertexBlockFromSize(mb.VertDef.entrySize, BigEndian, VertBlockData);
				v.position = block.position;
				v.normal = block.normal;
                v.texCoord = block.texCoord;
                vertices1[i] = v;
            }

            Log("Vert offset: " + std::to_string(mb.VertDef.vertexOffset), EType::PURPLE);
            Log("Vertex count: " + std::to_string(mb.VertDef.vertexCount), EType::Warning);
            Log("Entry size: " + std::to_string(mb.VertDef.entrySize), EType::BLUE);
            Log("Model Index: " + std::to_string(y), EType::GREEN);

            int IndicesOffset = 0;
            if (mb.IndiceDef.IndicesOffset != 0) {
                IndicesOffset = mb.IndiceDef.IndicesOffset;
            }
            else {
                if (mb.IndiceDef.IndicesOffset2 != 0) {
                    if (mb.IndiceDef.IndicesOffset3 != 0) {
                        if (mb.IndiceDef.IndicesOffset3 < mb.IndiceDef.IndicesOffset2) {
                            IndicesOffset = mb.IndiceDef.IndicesOffset3;
                        }
                        else {
                            IndicesOffset = mb.IndiceDef.IndicesOffset2;
                        }
                    }
                    else {
                        IndicesOffset = mb.IndiceDef.IndicesOffset2;
                    }
                }
                else if (mb.IndiceDef.IndicesOffset3 != 0) {
                    IndicesOffset = mb.IndiceDef.IndicesOffset3;
                }
            }

            Log("IndicesOffset: " + std::to_string(IndicesOffset), EType::PURPLE);

            std::vector<uint32_t> indices1;
            int CurrentIndiceOffset = IndicesOffset;
            while (true) {
                if (CurrentIndiceOffset + 5 >= VGPU.size()) {
                    Log("Reached end of VGPU data or next vertex offset, stopping indice extraction.", EType::Error);
                    break;
                }
                else if (verts.size() - 1 >= y + 1) {
                    if (CurrentIndiceOffset + 5 >= GetNextVertexBlock(verts[y], verts).vertexOffset) {
                        Log("Reached next vertex offset, stopping indice extraction.", EType::Error);
                        break;
                    }
                }

                uint16_t index1;
                memcpy(&index1, &VGPU[CurrentIndiceOffset], sizeof(uint16_t));
                if (BigEndian) {
                    index1 = _byteswap_ushort(index1);
                }
                uint16_t index2;
                memcpy(&index2, &VGPU[CurrentIndiceOffset + 2], sizeof(uint16_t));
                if (BigEndian) {
                    index2 = _byteswap_ushort(index2);
                }
                uint16_t index3;
                memcpy(&index3, &VGPU[CurrentIndiceOffset + 4], sizeof(uint16_t));
                if (BigEndian) {
                    index3 = _byteswap_ushort(index3);
                }

                if (index1 == 0 && index2 == 0 && index3 == 0) {
                    Log("Reached 0,0,0 indice, stopping indice extraction.", EType::Error);
                    break;
                }

                indices1.push_back(index1);
                indices1.push_back(index2);
                indices1.push_back(index3);
                CurrentIndiceOffset += 6;
            }
            Log("Indices count: " + std::to_string(indices1.size()), EType::PURPLE);
            Log("End of Indices: " + std::to_string(CurrentIndiceOffset), EType::PURPLE);


            std::string desktopPath;
            char path[MAX_PATH];
            if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_DESKTOP, nullptr, 0, path))) {
                desktopPath = std::string(path);
            }

            exportOBJ(vertices1, indices1, desktopPath + "/model_" + std::to_string(y) + ".obj");

            //If model_MOD.obj Exists, make a modded vgpu file with the new vertices
            std::string moddedFilePath = desktopPath + "/model_" + std::to_string(y) + "_MOD.obj";
            if (std::filesystem::exists(moddedFilePath)) {
                std::vector<Vector3> moddedVerts;
                std::ifstream moddedFile(moddedFilePath);
                if (moddedFile.is_open()) {
                    std::string line;
                    while (std::getline(moddedFile, line)) {
                        if (line.substr(0, 2) == "v ") { // Vertex line
                            Vector3 v;
                            sscanf(line.c_str(), "v %f %f %f", &v.x, &v.y, &v.z);
                            if (BigEndian) {
                                v.x = byteswap_float(v.x);
                                v.y = byteswap_float(v.y);
                                v.z = byteswap_float(v.z);
                            }
                            moddedVerts.push_back(v);
                        }
                    }
                    moddedFile.close();
                    // Replace vertices in VGPU with modded vertices
                    vBYTES newVGPU = {};
                    newVGPU = ReplaceVertsInVGPU(VGPU, mb.VertDef, moddedVerts);
                    std::ofstream newVgpuFile(desktopPath + "/model_" + std::to_string(y) + "_MOD.vgpu", std::ios::binary);
                    if (newVgpuFile.is_open()) {
                        newVgpuFile.write(reinterpret_cast<const char*>(newVGPU.data()), newVGPU.size());
                        newVgpuFile.close();
                        Log("Modded VGPU file created: " + desktopPath + "/model_" + std::to_string(y) + "_MOD.vgpu", EType::GREEN);
                    }
                    else {
                        Log("Failed to create modded VGPU file.", EType::Error);
                    }
                }
            }

            y++;
        }
    }


        
}
