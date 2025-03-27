#pragma once

#include <string>
#include "Utils/ini.h"



inline static void ShowPopup(std::string PopupMessage, bool HasProgressBar){
	//Add Bool to Ini
	mINI::INIFile file("settings.ini");
	mINI::INIStructure ini;
	file.read(ini);
	ini["Settings"]["ShowPopup"] = "true";
	ini["Settings"]["PopupMessage"] = PopupMessage;
	ini["Settings"]["HasProgressBar"] = HasProgressBar ? "true" : "false";
	file.write(ini);
	
}

inline static void ClosePopup(){
	//Add Bool to Ini
	mINI::INIFile file("settings.ini");
	mINI::INIStructure ini;
	file.read(ini);
	ini["Settings"]["ShowPopup"] = "false";
	file.write(ini);

}

inline static void UpdateProgressBar(float Progress){
	//Add Bool to Ini
	mINI::INIFile file("settings.ini");
	mINI::INIStructure ini;
	file.read(ini);
	int ProgressInt = Progress * 100;
	ini["Settings"]["Progress"] = std::to_string(ProgressInt);
	file.write(ini);

}