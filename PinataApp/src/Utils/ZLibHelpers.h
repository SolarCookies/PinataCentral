#pragma once

#include <string>
#include <vector>
#include "zlib.h"
#include <iostream>

#include "Log.hpp"

#define BYTE unsigned char
#define BYTES std::vector<BYTE>

namespace Zlib {

	//calculates the checksum of a CAFF header (Where the checksum bytes are nulled)
	//CREDIT TO MOJOBOJO FOR THIS AWESOME, WORKING HEADER CHECKSUM RECALCULATOR :D 
	//Copyed from https://github.com/weighta/Mumbos-Motors
	inline static uint32_t CAFF_checksum(BYTES CAFFHeader) {
		uint32_t r11 = 0;

		for (size_t i = 0; i < CAFFHeader.size(); i++) {
			uint32_t r8 = CAFFHeader[i];
			uint32_t r10 = r11 << 4;

			if ((r8 & 0x80) > 0) {
				r11 = 0xFFFFFF80 | r8;
			}
			else {
				r11 = r8;
			}

			r11 = r11 + r10;
			r10 = r11 & 0xF0000000;

			if (r10 != 0) {
				r8 = r10 >> 24;
				r10 = r8 | r10;
				r11 = r10 ^ r11;
			}
		}

		return r11;
	}

	// Compresses data based on input size
	inline static BYTES CompressData(BYTES& data) {

		BYTES Result;
		const size_t Buffer_Size = 128 * 1024;
		Byte Temp_Buffer[Buffer_Size];

		z_stream Stream = {};
		Stream.next_in = (Bytef*)data.data();
		Stream.avail_in = data.size();
		Stream.next_out = Temp_Buffer;
		Stream.avail_out = Buffer_Size;

		deflateInit(&Stream, Z_DEFAULT_COMPRESSION);

		while (Stream.avail_in != 0) {
			if (deflate(&Stream, Z_NO_FLUSH) != Z_OK) {
				Log("Error initializing Zlib Compression!", EType::Error);
				return Result;
			}
			if (Stream.avail_out == 0) {
				Result.insert(Result.end(), Temp_Buffer, Temp_Buffer + Buffer_Size);
				Stream.next_out = Temp_Buffer;
				Stream.avail_out = Buffer_Size;
			}
		}

		int Deflate_Res = Z_OK;
		while (Deflate_Res == Z_OK) {
			if (Stream.avail_out == 0) {
				Result.insert(Result.end(), Temp_Buffer, Temp_Buffer + Buffer_Size);
				Stream.next_out = Temp_Buffer;
				Stream.avail_out = Buffer_Size;
			}
			Deflate_Res = deflate(&Stream, Z_FINISH);
		}

		if (Deflate_Res != Z_STREAM_END) {
			Log("Compression Error!", EType::Error);
			return Result;
		}

		Result.insert(Result.end(), Temp_Buffer, Temp_Buffer + Buffer_Size - Stream.avail_out);
		deflateEnd(&Stream);

		return Result;
	}

    // Decompresses data based on input size
    inline static BYTES DecompressData(BYTES& data, uint32_t DecompressSize) {
        if (DecompressSize == data.size()) {
            return data;
        }
		if (data.size() > DecompressSize) {
			Log("Compressed Size is greater than Decompress Size..", EType::Error);
			return {};
		}

        BYTES Result(DecompressSize);

        z_stream Stream = {};
        Stream.avail_in = static_cast<uInt>(data.size());
        Stream.next_in = (Bytef*)data.data();
        Stream.avail_out = static_cast<uInt>(Result.size());
        Stream.next_out = Result.data();

        if (inflateInit(&Stream) != Z_OK) {
			Log("Error initializing Zlib Decompression!", EType::Error);
            return Result;
        }

        int Ret = inflate(&Stream, Z_FINISH);
        if (Ret != Z_STREAM_END) {
            inflateEnd(&Stream);
			Log("Zlib Decompression Error!", EType::Error);
			Log("Error Code: " + std::to_string(Ret), EType::Error);
            if (Stream.msg != nullptr) {
				Log("Error Message: " + std::string((char*)Stream.msg), EType::Error);
            }
			if (Ret == Z_BUF_ERROR) {
				Log("Buffer Error: Not enough room in the output buffer!", EType::Error);
			}

            return Result;
        }

        inflateEnd(&Stream);

        if (Stream.total_out != DecompressSize) {
            Result.resize(Stream.total_out);
        }

        return Result;
    }

	// For 32 bit/4 byte ints
	inline static uint32_t ConvertBytesToInt(BYTES& data, bool isBigEndian) {
		if (data.size() != sizeof(uint32_t)) {
			return 0;
		}

		uint32_t value;
		memcpy(&value, data.data(), sizeof(uint32_t));

		if (isBigEndian) {
			value = _byteswap_ulong(value);
		}

		return value;
	}

	inline static uint32_t ConvertBytesToInt(BYTES& data, uint32_t startOffset, bool isBigEndian) {
		if (data.size() < startOffset + sizeof(uint32_t)) {
			return 0;
		}

		uint32_t value;
		memcpy(&value, data.data() + startOffset, sizeof(uint32_t));

		if (isBigEndian) {
			value = _byteswap_ulong(value);
		}

		return value;
	}

	// For 32 bit/4 byte floats
	inline static float ConvertBytesToFloat(BYTES& data, bool isBigEndian) {
		if (data.size() != sizeof(float)) {
			return 0;
		}

		float value;
		memcpy(&value, data.data(), sizeof(float));

		if (isBigEndian) {
			uint32_t temp = _byteswap_ulong(*reinterpret_cast<uint32_t*>(&value));
			value = *reinterpret_cast<float*>(&temp);
		}

		return value;
	}

	inline static float ConvertBytesToFloat(BYTES& data, uint32_t startOffset, bool isBigEndian) {
		if (data.size() < startOffset + sizeof(float)) {
			return 0;
		}

		float value;
		memcpy(&value, data.data() + startOffset, sizeof(float));

		if (isBigEndian) {
			uint32_t temp = _byteswap_ulong(*reinterpret_cast<uint32_t*>(&value));
			value = *reinterpret_cast<float*>(&temp);
		}

		return value;
	}

	// For 16 bit/2 byte ints
	inline static int_least16_t ConvertBytesToShort(BYTES& data, bool isBigEndian) {
		if (data.size() != sizeof(int_least16_t)) {
			return 0;
		}

		int_least16_t value;
		memcpy(&value, data.data(), sizeof(int_least16_t));

		if (isBigEndian) {
			value = _byteswap_ushort(value);
		}

		return value;
	}

	inline static int_least16_t ConvertBytesToShort(BYTES& data, uint32_t startOffset, bool isBigEndian) {
		if (data.size() < startOffset + sizeof(int_least16_t)) {
			return 0;
		}

		int_least16_t value;
		memcpy(&value, data.data() + startOffset, sizeof(int_least16_t));

		if (isBigEndian) {
			value = _byteswap_ushort(value);
		}

		return value;
	}

	// Converts bytes to ascii string
	inline static std::string ConvertBytesToString(BYTES& data) {
		return std::string(data.begin(), data.end());
	}

	// For 32 bit/4 byte ints
	inline static BYTES ConvertIntToBytes(uint32_t value, bool isBigEndian) {
		if (isBigEndian) {
			value = _byteswap_ulong(value);
		}

		BYTES data(sizeof(uint32_t));
		memcpy(data.data(), &value, sizeof(uint32_t));
		return data;
	}

	// For 32 bit/4 byte floats
	inline static BYTES ConvertFloatToBytes(float value, bool isBigEndian) {
		if (isBigEndian) {
			uint32_t temp = _byteswap_ulong(*reinterpret_cast<uint32_t*>(&value));
			value = *reinterpret_cast<float*>(&temp);
		}

		BYTES data(sizeof(float));
		memcpy(data.data(), &value, sizeof(float));
		return data;
	}

	// For 16 bit/2 byte ints
	inline static BYTES ConvertShortToBytes(int_least16_t value, bool isBigEndian) {
		if (isBigEndian) {
			value = _byteswap_ushort(value);
		}

		BYTES data(sizeof(int_least16_t));
		memcpy(data.data(), &value, sizeof(int_least16_t));
		return data;
	}

}
