#pragma once
#include <string>
#include "Walnut/Image.h"

//Uses texconv.exe to convert a dds file to a png file then loads it up using walnut::Image
namespace dds
{
	//Converts a dds file to a png file
	inline void ConvertDDS(const std::string& ddsPath, const std::string& pngPath)
	{
		//texconv.exe path.dds -ft png -o path.png
		std::string command = "Assets/texconv.exe " + ddsPath + " -ft png -o " + pngPath + " -y";
		system(command.c_str());
	}

	//Loads a dds file into a walnut::Image
	inline Walnut::Image LoadDDS(const std::string& ddsPath)
	{
		ConvertDDS(ddsPath, "./");
		return Walnut::Image("Assets/temp.PNG");
	}

}

namespace raw {

	std::shared_ptr<Walnut::Image> m_FinalImage;


	//Loads a raw file into a walnut::Image
	inline void LoadRAW(BYTES VDAT,BYTES VGPU, bool IsBigEndian)
	{
		//Width and Height are shorts stored at 8 and 10
		uint16_t Width = Zlib::ConvertBytesToShort(VDAT, 8, IsBigEndian);
		uint16_t Height = Zlib::ConvertBytesToShort(VDAT, 10, IsBigEndian);

		if (m_FinalImage)
		{
			// No resize necessary
			if (m_FinalImage->GetWidth() == Width && m_FinalImage->GetHeight() == Height)
				return;

			m_FinalImage->Resize(Width, Height);
		}
		else
		{
			m_FinalImage = std::make_shared<Walnut::Image>(Width, Height, Walnut::ImageFormat::RGBA);
		}

		uint32_t* m_ImageData = nullptr;

		m_ImageData = new uint32_t[Width * Height];

		//VGPU is raw pixel data with no header that contains rgba
		//for each pixel we have 4 bytes
		for (int i = 0; i < VGPU.size(); i += 4)
		{
			//RGBA
			BYTE R = VGPU[i];
			BYTE G = VGPU[i + 1];
			BYTE B = VGPU[i + 2];
			BYTE A = VGPU[i + 3];

			uint32_t color = (A << 24) | (B << 16) | (G << 8) | R;
			m_ImageData[i / 4] = color;

		}

		m_FinalImage->SetData(m_ImageData);
	}
}