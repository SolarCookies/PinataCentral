#pragma once

#include <string>
#include "Utils/ini.h"
#include <filesystem>

inline static std::string RootDicrectory = ""; // Contains the main executable to the game
inline static std::string PackagesDicrectory = ""; // Contains all assets in the game
inline static std::string BundlesDicrectory = ""; // Contains localization
inline static std::string DebugPackDicrectory = ""; // Contains all assets in the game unknown as to why its duplicated
inline static std::string WadDicrectory = ""; // Only used on Windows port or on the fly recompiling shaders with graphic settings
inline static std::string SaveDirectory = ""; // Contains save files for the game (Only PC port at the moment due to TIP using Xenia Directory)
inline static std::string GardenDirectory = "";

inline static void SaveSettings() {
	// Save all settings to the ini file
	mINI::INIFile file("settings.ini");
	mINI::INIStructure ini;
	file.read(ini);
	ini["Settings"]["Root_Dicrectory"] = RootDicrectory;
	ini["Settings"]["Packages_Dicrectory"] = PackagesDicrectory;
	ini["Settings"]["Bundles_Dicrectory"] = BundlesDicrectory;
	ini["Settings"]["DebugPack_Dicrectory"] = DebugPackDicrectory;
	ini["Settings"]["Wad_Dicrectory"] = WadDicrectory;
	ini["Settings"]["Save_Directory"] = SaveDirectory;
	ini["Settings"]["Garden_Directory"] = GardenDirectory;
	file.write(ini);
}

inline static void LoadSettings() {
	// Load all settings from the ini file
	mINI::INIFile file("settings.ini");
	mINI::INIStructure ini;
	file.read(ini);
	RootDicrectory = ini["Settings"]["Root_Dicrectory"].empty() ? "" : ini["Settings"]["Root_Dicrectory"];
	PackagesDicrectory = ini["Settings"]["Packages_Dicrectory"].empty() ? "" : ini["Settings"]["Packages_Dicrectory"];
	BundlesDicrectory = ini["Settings"]["Bundles_Dicrectory"].empty() ? "" : ini["Settings"]["Bundles_Dicrectory"];
	DebugPackDicrectory = ini["Settings"]["DebugPack_Dicrectory"].empty() ? "" : ini["Settings"]["DebugPack_Dicrectory"];
	WadDicrectory = ini["Settings"]["Wad_Dicrectory"].empty() ? "" : ini["Settings"]["Wad_Dicrectory"];
	SaveDirectory = ini["Settings"]["Save_Directory"].empty() ? "" : ini["Settings"]["Save_Directory"];
	GardenDirectory = ini["Settings"]["Garden_Directory"].empty() ? "" : ini["Settings"]["Garden_Directory"];
}

inline static void UpdateRootDicrectory(std::string newPath) {
	RootDicrectory = newPath;

	if (std::filesystem::exists(RootDicrectory + "/Viva Pinata.exe") && std::filesystem::exists(RootDicrectory + "/bundles_packages/1.pkg")) {
		//PC
		PackagesDicrectory = RootDicrectory + "\\bundles_packages";
		BundlesDicrectory = RootDicrectory + "\\bundles";
		DebugPackDicrectory = RootDicrectory + "\\debug";
		WadDicrectory = RootDicrectory + "\\datacx";
		std::string appDataPath = std::getenv("LOCALAPPDATA") ? std::getenv("LOCALAPPDATA") : "";
		SaveDirectory = appDataPath + "\\Saved Games" + "\\Microsoft Games" + "\\Viva Pinata";
		GardenDirectory = SaveDirectory + "\\Saves";
	}
	else if (std::filesystem::exists(RootDicrectory + "/default.xex") && std::filesystem::exists(RootDicrectory + "/Beta/packages/1.pkg")) {
		//TIP
		PackagesDicrectory = RootDicrectory + "\\Beta\\packages";
		BundlesDicrectory = RootDicrectory + "\\Beta\\bundles";
		DebugPackDicrectory = RootDicrectory + "\\Beta\\packed";
		WadDicrectory = "";
		SaveDirectory = ""; // TIP uses Xenia's save directory which is not configurable from here
		GardenDirectory = "";
	}

	SaveSettings();
}

inline static void ResetSettings() {
	RootDicrectory = "";
	PackagesDicrectory = "";
	BundlesDicrectory = "";
	DebugPackDicrectory = "";
	WadDicrectory = "";
	SaveSettings();
}

inline static bool IsTIP() {
	return std::filesystem::exists(RootDicrectory + "/default.xex") && std::filesystem::exists(RootDicrectory + "/Beta/packages/1.pkg");
}
