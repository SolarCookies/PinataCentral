#pragma once
#include <string>
#include <vector>
#include "OpenFIleDialog.h"
#include <imgui.h>


using namespace std;

// Add a Array of strings
vector<string> Logs;
vector<string> Keys;

bool ForceDown = false;


enum EType {
	Normal,
	Warning,
	Error,
	GREEN,
	BLUE,
	PURPLE
};


//Logs a Int
void Log(int i)
{
	Logs.push_back(to_string(i));
	Keys.push_back(" ");
	ForceDown = true;
}

//Logs a uint32_t
void Log(uint32_t i)
{
	Logs.push_back(to_string(i));
	Keys.push_back(" ");
	ForceDown = true;
}

//Logs a Float
void Log(float f)
{
	Logs.push_back(to_string(f));
	Keys.push_back(" ");
	ForceDown = true;
}
//Logs a Double
void Log(double d)
{
	Logs.push_back(to_string(d));
	Keys.push_back(" ");
	ForceDown = true;
}
//Logs a Bool
void Log(bool b)
{
	Logs.push_back(to_string(b));
	Keys.push_back(" ");
	ForceDown = true;
}
//Logs a Char
void Log(char c)
{
	Logs.push_back(to_string(c));
	Keys.push_back(" ");
	ForceDown = true;
}
//Logs a ImGui Vector2
void Log(ImVec2 v)
{
	Logs.push_back("X: " + to_string(v.x) + " Y: " + to_string(v.y));
	Keys.push_back(" ");
	ForceDown = true;
}
//Logs a ImGui Vector3
void Log(ImVec4 v)
{
	Logs.push_back("X: " + to_string(v.x) + " Y: " + to_string(v.y) + " Z: " + to_string(v.z));
	Keys.push_back(" ");
	ForceDown = true;
}

void Log(BYTES Bytes)
{
	string str = "";
	for (int i = 0; i < Bytes.size(); i++)
	{
		//use mod to add a space every 4 bytes
		if (i % 4 == 0 && i != 0) {
			str += " | ";
		}
		//format the byte to hex numbers and letters
		str += " " + to_string(Bytes[i] >> 4) + to_string(Bytes[i] & 0xF);
	}
	Logs.push_back(str);
	Keys.push_back("PURPLE");
}

//Clears the log
void ClearLog()
{
	Logs.clear();
	Keys.clear();
	ForceDown = true;
}

//Draws the log using ImGui
void DrawLog()
{
	ImGui::BeginChild("Log");
	for (int i = 0; i < Logs.size(); i++)
	{
		if (Keys[i] == "Err"){
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
		}else if (Keys[i] == "Warn"){
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
		}else if (Keys[i] == "GREEN") {
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
		}else if (Keys[i] == "BLUE") {
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 255, 255));
		}else if (Keys[i] == "PURPLE") {
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 255, 255));
		}
		else {
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
		}
		ImGui::Text(Logs[i].c_str());
		ImGui::PopStyleColor();
		//force scroll down
		if (ForceDown) {
			ImGui::SetScrollHereY(-9999.0f); //-999.0f is a placeholder length until
			ForceDown = false;
		}

	}
	ImGui::EndChild();
	
}

//Removes the last log
void RemoveLastLog()
{
	Logs.pop_back();
	Keys.pop_back();
	ForceDown = true;
}

//Removes a specific log
void RemoveFromLog(int i)
{
	Logs.erase(Logs.begin() + i);
	Keys.erase(Keys.begin() + i);
	ForceDown = true;
}

//Uses a Key to only make sure item is logged once (Useful for logging values that change every frame)
void KeyLogString(string Key, string Value)
{
	bool Found = false;

	for (int i = 0; i < Keys.size(); i++)
	{
		if (Keys[i] == Key)
		{
			RemoveFromLog(i);
			Logs.push_back(Value);
			Keys.push_back(Key);
			Found = true;
			ForceDown = true;
			break;
		}
		
	}
	if (!Found) {

		Logs.push_back(Value);
		Keys.push_back(Key);
		ForceDown = true;

	}
}



//Logs a String
void Log(string str, EType Type)
{
	switch (Type)
	{
	case Normal:
		Logs.push_back(str);
		Keys.push_back(" ");
		break;
	case Warning:
		Logs.push_back(str);
		Keys.push_back("Warn");
		break;
	case Error:
		Logs.push_back(str);
		Keys.push_back("Err");
		break;
	case GREEN:
		Logs.push_back(str);
		Keys.push_back("GREEN");
		break;
	case BLUE:
		Logs.push_back(str);
		Keys.push_back("BLUE");
		break;
	case PURPLE:
		Logs.push_back(str);
		Keys.push_back("PURPLE");
		break;
	}
	ForceDown = true;
}



