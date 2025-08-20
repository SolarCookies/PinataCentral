#pragma once
#include <windows.h>
#include <wincrypt.h>
#include <fstream>
#include <vector>
#include <iostream>

#pragma comment(lib, "advapi32.lib")

#pragma pack(push, 1)
struct RNCEHeader {
    char magic[4]; // "RCNE"
    uint32_t Size;
    uint32_t unk1;
    uint32_t unk2;
};

struct PVtHeader {
    char magic[4]; // "11PV"
    uint32_t FirstFileOffset;
    uint32_t SecFileOffset;
    char unk1[24]; //Likely checksums
    char HashKey[16]; //Hash key always at offset 8268 in the file
    char unk2[24]; //Normally empty
};
#pragma pack(pop)

// EXE hash from .rdata section
inline static BYTE ExeSaveHash[16] = {
    0x50, 0xFA, 0xBE, 0xD5, 0xCA, 0xBB, 0xA9, 0xE5,
    0xCA, 0x5C, 0xAD, 0xED, 0xD1, 0x5C, 0x0D, 0xAD
};

// Example XUID
inline static uint32_t XUID[2] = { 0xDEADBEEF, 0xE0000000 };

// Reads 16 bytes from save at offset 8268
inline static bool ReadSaveHash(const std::wstring& filename, BYTE* outHash)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file) return false;
    file.seekg(8268);
    file.read(reinterpret_cast<char*>(outHash), 16);
    return true;
}

// Global provider handle (mirrors game's static)
static HCRYPTPROV g_hProv = 0;

inline static bool AcquireCryptoProvider()
{
    if (g_hProv) return true;

    static const char* kProvName = "Microsoft Enhanced RSA and AES Cryptographic Provider";
    DWORD flags = 0;
    int tries = 0;
    bool initProblem = false;

    while (tries++ < 10)
    {
        if (CryptAcquireContextA(&g_hProv, /*szContainer*/ nullptr, kProvName,
            PROV_RSA_AES, flags))
            return true;

        DWORD err = GetLastError();
        // Retry/backoff behavior similar to the game
        if (err == NTE_BAD_KEYSET || err == NTE_KEYSET_NOT_DEF) {
            flags |= CRYPT_NEWKEYSET;                 // try creating keyset
        }
        else if (err == NTE_PROVIDER_DLL_FAIL ||
            err == NTE_PROV_DLL_NOT_FOUND ||
            err == NTE_PROV_TYPE_NOT_DEF) {
            initProblem = true;
        }
        else if (err == ERROR_BUSY || err == ERROR_NOT_ENOUGH_MEMORY) {
            Sleep(100);
        }
        // small delay between attempts
        Sleep(50);
    }

    // In the game they show a MessageBox on persistent provider init issues.
    if (initProblem) {
        // Optional: log something here.
    }
    g_hProv = 0;
    return false;
}

// Generate AES key from hash using CryptoAPI
inline static bool CreateAESKeyFromHash(const BYTE* buf, DWORD len, HCRYPTKEY& hKey, DWORD* outBlockLenBytes = nullptr)
{
    const BYTE* buff = buf;

    hKey = 0;
    if (!AcquireCryptoProvider()) {
        std::wcerr << L"AcquireCryptoProvider failed: " << GetLastError() << L"\n";
        return false;
    }

    if (!CryptCreateHash(g_hProv, 32771u, 0, 0, (HCRYPTHASH*)&buf)) {// 32771u → CALG_SHA1 (SHA-1 hash algorithm)
        std::wcerr << L"CryptCreateHash failed: " << GetLastError() << L"\n";
        return false;
    }


    BOOL ok = CryptHashData((HCRYPTHASH)buf, buff, 40u, 0);
    if (ok) {
        //   CryptDeriveKey(g_hProv, 26126u, phHash, 8388608u, phKey);
        ok = CryptDeriveKey(g_hProv, 26126u, (HCRYPTHASH)buf, 8388608u, &hKey);
    }

    // Clean up hash either way
    //CryptDestroyHash(hHash);

    if (!ok) {
        std::wcerr << L"CryptDeriveKey failed: " << GetLastError() << L"\n";
        if (hKey) { CryptDestroyKey(hKey); hKey = 0; }
        return false;
    }

    BYTE pbdata[4];
	DWORD dwBlockLen = 4;
    if (!CryptGetKeyParam(hKey, 8u, pbdata, &dwBlockLen, 0)) {
        std::wcerr << L"CryptSetKeyParam(KP_MODE) failed: " << GetLastError() << L"\n";
        CryptDestroyKey(hKey);
        CryptReleaseContext(g_hProv, 0);
        return false;
    }
	outBlockLenBytes = &dwBlockLen;

    return true;
}

// Decrypt ENCR block
inline static bool DecryptBlock(HCRYPTKEY hKey, std::vector<BYTE>& buffer, int expectedSize)
{
    if (!hKey || buffer.empty()) return false;

    

    DWORD dataLen = static_cast<DWORD>(buffer.size()); // in/out length
	DWORD expectedLen = static_cast<DWORD>(16);
    if (!CryptDecrypt(hKey, 0, 0, 0, buffer.data(), &expectedLen)) {
        std::wcerr << L"CryptDecrypt failed: " << GetLastError() << L"\n";
        return false;
    }
    buffer.resize(dataLen); // trimmed to plaintext
    return true;
}

inline static bool ReadRNCEBlock(std::ifstream& file, size_t offset, std::vector<char>& outData, RNCEHeader& header)
{
    int headeroffset = offset - 16;
    file.seekg(headeroffset);
    std::cout << "Reading RNCE block at offset: " << headeroffset << "\n";
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (!file) return false;

    outData.resize(header.Size);
    file.read(outData.data(), header.Size);
    return file.good();
}


inline static int ReadSaveFile(const std::wstring& saveFile)
{
    // Step 1: Read the save hash (offset 8268)
    BYTE saveHash[16];
    if (!ReadSaveHash(saveFile, saveHash)) {
        std::wcerr << L"Failed to read save hash\n";
        return 1;
    }
    
    // Step 2: Build combined 40-byte hash: SaveHash + ExeSaveHash + XUID
    BYTE combinedHash[40] = { 0x17,0x2E,0xE0,4,0x4D,0x71,0x50,0x4B,0xBF,0xA0
                            ,0x43,0x4F,0x70,0x24,0x11,0x67,0x50,0xFA,0xBE,0xD5
                            ,0xCA,0xBB,0xA9,0xE5,0xCA,0x5C,0xAD,0xED,0xD1,0x5C
                            ,0xD,0xAD,0xEE,0xDB,0xEA,0xD,0x0,0x0,0x0,0xE };
                            
                            // HCRYPTKEY hKey[2] = { 0x4DDF728u, 0x10u };

    
    //BYTE combinedHash[40];
    //memcpy(combinedHash, saveHash, 16);
    //memcpy(combinedHash + 16, ExeSaveHash, 16);
    //memcpy(combinedHash + 32, XUID, 8);


    // Step 3: Generate AES key
    HCRYPTKEY hKey = 0;
	DWORD outBlockLenBytes;
    if (!CreateAESKeyFromHash(combinedHash, 40, hKey, &outBlockLenBytes)) {
        std::wcerr << L"Failed to create AES key\n";
        return 1;
    }

	std::cout << "AES key created successfully, block length: " << outBlockLenBytes << "\n";

    std::ifstream file(saveFile, std::ios::binary);
    if (!file) {
        std::wcerr << L"Failed to open save file\n";
        CryptDestroyKey(hKey);
        return 1;
    }

    // Step 4: Read PVtHeader at offset 8232
    PVtHeader pvtHeader;
    file.seekg(8232);
    file.read(reinterpret_cast<char*>(&pvtHeader), sizeof(pvtHeader));
    if (!file) {
        std::wcerr << L"Failed to read PVtHeader\n";
        CryptDestroyKey(hKey);
        return 1;
    }

    // Helper lambda to decrypt and write RNCE block
    auto decryptAndWrite = [&](size_t offset, const std::string& outName) -> bool {
        std::vector<char> rnceData;
        RNCEHeader header;
        if (!ReadRNCEBlock(file, offset, rnceData, header)) {
            std::cout << "Failed to read RNCE block at offset: " << offset << "\n";
            return false;
        }

        std::vector<BYTE> buffer(rnceData.begin(), rnceData.end());
        if (!DecryptBlock(hKey, buffer, header.unk1)) {
            std::cout << "Decryption failed for " << outName << "\n";
            std::cout << "Error code: " << GetLastError() << "\n";
            std::cout << "Key: " << hKey << "\n";
            return false;
        }

        std::string desktopPath = std::string(getenv("USERPROFILE")) + "\\Desktop\\" + outName;
        std::ofstream out(desktopPath, std::ios::binary);
        out.write(reinterpret_cast<char*>(buffer.data()), buffer.size());
        std::cout << desktopPath << " decrypted, size: " << buffer.size() << "\n";
        return true;
        };

    // Step 5: Iterate through RNCE blocks (example: 3 blocks after SecFileOffset)
    size_t offset = pvtHeader.SecFileOffset;
    for (int i = 1; i <= 3; ++i) {
        RNCEHeader header;
        file.seekg(offset);
        file.read(reinterpret_cast<char*>(&header), sizeof(header));
        if (!file) break;

        decryptAndWrite(offset + sizeof(RNCEHeader), "file" + std::to_string(i) + ".bin");
        offset += sizeof(RNCEHeader) + header.Size; // Move to next block
    }

    CryptDestroyKey(hKey);
    std::wcout << L"Decryption complete!\n";
    return 0;
}