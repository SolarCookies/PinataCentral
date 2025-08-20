#pragma once
#include <vector>
#include <string>
#include "../Utils/ZLibHelpers.h"
#include "../Utils/OpenFileDialog.h"
#include "../Utils/Log.hpp"
#include "../Utils/half.hpp"

struct LowVector2 {
	half_float::half u, v; // 4 bytes total
};
struct Vector3 {
	float x, y, z;
};
struct Vector2 {
	float u, v;
};


struct VertexBlock {
	Vector3 position;
	Vector3 normal;
	Vector2 texCoord;
	Vector3 VertexColor;
};


inline static float byteswap_float(float value) {
	uint32_t temp_int;
	// Copy the float's bytes to an integer of the same size
	std::memcpy(&temp_int, &value, sizeof(float));
	// Byteswap the integer
	temp_int = _byteswap_ulong(temp_int);
	float result;
	// Copy the bytes back to a float
	std::memcpy(&result, &temp_int, sizeof(float));
	return result;
}

inline static half_float::half byteswap_half(half_float::half value) {
	uint16_t temp_int;
	// Copy the half's bytes to an integer of the same size
	std::memcpy(&temp_int, &value, sizeof(half_float::half));
	// Byteswap the integer
	temp_int = _byteswap_ushort(temp_int);
	half_float::half result;
	// Copy the bytes back to a half
	std::memcpy(&result, &temp_int, sizeof(half_float::half));
	return result;
}

inline VertexBlock ConstructVertexBlockFromSize(int size, bool bigEndian, vBYTES block) {
	VertexBlock Vert;
	memcpy(&Vert.position, &block[0], sizeof(Vector3)); // Read Position (Always at the start of the block)

	if (bigEndian) { // TIP
		Vert.position.x = byteswap_float(Vert.position.x);
		Vert.position.y = byteswap_float(Vert.position.y);
		Vert.position.z = byteswap_float(Vert.position.z);

		if (size == 52) {
			LowVector2 lv;
			Vector2 uv;
			memcpy(&lv, &block[32], sizeof(LowVector2));
			if (bigEndian) {
				lv.u = byteswap_half(lv.u);
				lv.v = byteswap_half(lv.v);
			}
			uv.u = static_cast<float>(lv.u);
			uv.v = static_cast<float>(lv.v);
			Vert.texCoord = uv;
		}
		else if (size == 48) {
			LowVector2 lv;
			Vector2 uv;
			memcpy(&lv, &block[28], sizeof(LowVector2));

			lv.u = byteswap_half(lv.u);
			lv.v = byteswap_half(lv.v);

			uv.u = static_cast<float>(lv.u);
			uv.v = static_cast<float>(lv.v);
			Vert.texCoord = uv;
		}
	}
	else { // PC GLFW
		if( size == 76) {
			memcpy(&Vert.texCoord, &block[36], sizeof(Vector2));
		}
		else if (size == 48) {
			memcpy(&Vert.texCoord, &block[28], sizeof(Vector2));
		}
	}

	return Vert;
}