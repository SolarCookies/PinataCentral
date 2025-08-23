#include "aid_pip_pinata_misc_Widget.h"
#include "../AidEditor.h"
#include "LocalizationWidget.h"
#include "../../../Bundle.h"
#include "IntWidget.h"

void PipWidget::init(AidEditor& aidEditor, vBYTES& vdat, vBYTES& vgpu, BundleReader& bundle)
{
	int unk1 = 0;
	int unk2 = 0;
	char LocalizationName[136];
	//std::string Sound;
	int unk3 = 0;

	memcpy(&unk1, &vdat[0], 4);
	memcpy(&unk2, &vdat[4], 4);
	memcpy(&LocalizationName, &vdat[8], 136);
	//memcpy(&Sound, &vdat[144], 136);
	memcpy(&unk3, &vdat[280], 4);
	std::string LocalizationNameStr(LocalizationName);
	std::cout << "Localization Data: " << LocalizationNameStr << std::endl;

	int index = -1;
	bool found = false;
	for (auto& Name : bundle.bvref.ChunkNames) {

		std::string chunkNameFull = Name;
		size_t commaPos = chunkNameFull.find(',');
		std::string chunkName = (commaPos != std::string::npos) ? chunkNameFull.substr(0, commaPos) : chunkNameFull;

		index++;
		if (LocalizationNameStr == chunkName) {
			LocalizationNameStr = chunkName;
			found = true;
			break;
		}
	}
	if (found) {
		vBYTES vdat = caff::Get_VDAT(bundle.CAFFBYTES, bundle.bcaff, bundle.bvref);
		Localvdat.resize(bundle.bvref.ChunkInfos[index].VDAT_Size);
		std::memcpy(Localvdat.data(), vdat.data() + bundle.bvref.ChunkInfos[index].VDAT_Offset, bundle.bvref.ChunkInfos[index].VDAT_Size);
		std::cout << "Found Localization Data: " << LocalizationNameStr << std::endl;
		auto localizationWidget = std::make_unique<LocalizationWidget>();
		localizationWidget->setLocalizationData(Localvdat);
		aidEditor.AddWidget(localizationWidget.release());

		auto intWidget1 = std::make_unique<IntWidget>("Unk1", unk1);
		aidEditor.AddWidget(intWidget1.release());
		auto intWidget2 = std::make_unique<IntWidget>("Unk2", unk2);
		aidEditor.AddWidget(intWidget2.release());
		auto intWidget3 = std::make_unique<IntWidget>("Unk3", unk3);
		aidEditor.AddWidget(intWidget3.release());
	}
}
