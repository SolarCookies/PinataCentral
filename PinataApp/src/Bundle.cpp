#include "Bundle.h"
#include "PKG/pkg.h"

void BundleReader::ffread(std::string path, CAFF& caff, VREF& vref)
{
	//check if file exists
	if (!std::filesystem::exists(path)) {
		std::cout << "File does not exist: " << path << std::endl;
		return;
	}
	else {
		CAFFBYTES.clear();
		// Read the file into a buffer
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file) {
			std::cout << "Failed to open file: " << path << std::endl;
			return;
		}
		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		std::vector<uint8_t> buffer(size);
		if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
			std::cout << "Failed to read file: " << path << std::endl;
			return;
		}
		CAFFBYTES = vBYTES(buffer.begin(), buffer.end());

		//Bundles are little endian
		caff = caff::Read_Header(CAFFBYTES);

		vBYTES VREFB = caff::Get_VREF(CAFFBYTES, caff);

		//Debug pack is Big Endian initially with the CAFF header, however the vref is little endian
		vref = caff::Read_VREF(VREFB, caff, caff.IsBigEndian);

		for (size_t i = 0; i < vref.ChunkInfos.size(); ++i) {
			const ChunkInfo& chunk = vref.ChunkInfos[i];
		}



	}
}
