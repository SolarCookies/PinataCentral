#pragma once
#include <string>
#include <iostream>
#include <filesystem>
#include <vector>
#include "PKG/CAFF.h"





class BundleReader {
	public:
		BundleReader() = default;

		std::vector<unsigned char> CAFFBYTES;
		void ffread(std::string path, CAFF& caff, VREF& vref);

		VREF bvref;
		CAFF bcaff;
	BundleReader(const std::string& filePath) {
		if (!std::filesystem::exists(filePath)) {
			throw std::runtime_error("File does not exist: " + filePath);
		}
		ffread(filePath,bcaff, bvref);
	}
};

